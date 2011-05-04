
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>
#include <infiniband/verbs.h>
#include <errno.h>
#include <limits>
#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <vector>
#include <tr1/memory>
#include <memory>
#include <map>
#include <cmath>
#include <sstream>

#include "MirrorBufferSet.h"
#include "SocketHandle.h"
#include "ServerSocketHandle.h"
#include "PthreadFunctional.h"
#include "AutoLock.h"
#include "ErrnoException.h"


#define IBV_ACCESS_DEFAULT (IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE\
        | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC)

using namespace std;
using namespace std::tr1;

const double filt_a[] = { 1, -1.88903307939452, 0.894874344616636 };
const double filt_b[] = { 0.00146031630552773, 0.00292063261105546, 0.00146031630552773 };
const unsigned filt_len = 3;


std::ostream &operator<<(std::ostream &os, ibv_qp_state &state) {
    switch (state) {
    case IBV_QPS_RESET:
        os << "reset";
        break;
    case IBV_QPS_INIT:
        os << "init";
        break;
    case IBV_QPS_RTR:
        os << "rtr";
        break;
    case IBV_QPS_RTS:
        os << "rts";
        break;
    case IBV_QPS_SQD:
        os << "sqd";
        break;
    case IBV_QPS_SQE:
        os << "sqe";
        break;
    case IBV_QPS_ERR:
        os << "err";
        break;
    }
    return os;
}

template<typename T>
T Filter(T val, const T *b, unsigned b_len, const T *a, unsigned a_len, T *buf) {
    T out = val * b[0] + buf[0] * a[0];
    unsigned mlen = std::max(b_len, a_len);
    for (unsigned i = 1; i < mlen; ++i) {
        T v = buf[i];
        if (i < b_len) {
            v += val * b[i];
        }
        if (i < a_len) {
            v -= out * a[i];
        }
        buf[i-1] = v;
    }
    return out;
}

static double getTime() {
    timeval tv;
    if (gettimeofday(&tv, 0) != 0) {
        throw ErrnoException();
    }
    return static_cast<double>(tv.tv_sec) + 1e-6 * static_cast<double>(tv.tv_usec);
}

static void Sleep(double t) {
    timespec tsrec, tsrem;
    tsrec.tv_sec = (time_t)floor(t);
    tsrec.tv_nsec = (long)((t - floor(t))*1e9);
    while (nanosleep(&tsrec, &tsrem) != 0) {
        if (errno == EINTR) {
            tsrec = tsrem;
        } else {
            throw ErrnoException();
        }
    }
}

struct Bytes {
    Bytes(double v_) : v(v_) {}
    double v;
};

std::ostream &operator<<(std::ostream &os, const Bytes &b) {
    static const char *const P[] = { "B ", "kB", "MB", "GB", "TB" };
    int p = 0;
    double v = b.v;
    while (v >= 1000 && p < 5) {
        v /= 1000;
        p++;
    }
    os << setw(7) << setfill(' ') << setprecision(3) << fixed << v << " " << P[p];
    return os;
}

struct Time {
    Time(double v_) : v(v_) {}
    double v;
};

std::ostream &operator<<(std::ostream &os, const Time &t) {
    static const char *const P[] = { "s ", "ms", "us", "ns", "ps" };
    int p = 0;
    double v = t.v;
    while (v < 1 && p < 5) {
        v *= 1000;
        p++;
    }
    os << setw(7) << setfill(' ') << setprecision(3) << fixed << v << " " << P[p];
    return os;
}

void exchange_data(SocketHandle &sock, char *src, char *dst, unsigned len) {
    unsigned num_written = 0;
    while (num_written < len) {
        num_written += sock.Write(src + num_written, len - num_written);
    }
    unsigned num_read = 0;
    while (num_read < len) {
        unsigned nr = sock.Read(dst + num_read, len - num_read);
        if (nr == 0 && sock.Eof()) {
            throw std::runtime_error("Unexpected EOF encountered on socket.");
        }
        num_read += nr;
    }
}

struct QueueModel {
    QueueModel() : base(0), length(0), head(0), tail(0) {}
    QueueModel(char *b, uint64_t s)
        : base(b), length(s), head(0), tail(0)
    {}
    char *GetTail(uint64_t offset = 0) { return &base[(tail+offset)%length]; }
    char *GetHead(uint64_t offset = 0) { return &base[(head+offset)%length]; }
    void AdvanceTail(uint64_t count) {
        tail += count;
        while (tail > length) {
            tail -= length;
            head -= length;
        }
    }
    void AdvanceHead(uint64_t count) { head += count; }
    uint64_t Count() { return head - tail; }
    bool Empty() { return head == tail; }
    bool Full() { return Count() == Size(); }
    uint64_t Size() { return length; }
    uint64_t Freespace() { return length - Count(); }

    char *base;
    uint64_t length, head, tail;
};

struct MemQueue : public QueueModel {
    MemQueue(uint64_t size)
        : QueueModel((char*)malloc(size), size)
    {}
    ~MemQueue() {
        free(base);
    }
};

class Queue {
    enum State {
        s_init,
        s_running,
        s_stopped,
        s_error
    };
    enum MessageType {
        mt_update_queue,
        mt_dequeue,
        mt_enqueue
    };
    struct Message {
        MessageType type;
        union Data {
            struct UpdateQueue {
                uint64_t length, head, tail;
                uint32_t rqkey;
                char *base;
            } update_queue;
            struct Dequeue {
                uint64_t amount;
            } dequeue;
            struct Enqueue {
                uint64_t amount;
            } enqueue;
        } data;
    };

    struct PortInfo {
        int lid;
        int qp_num;
    };
public:
    enum Mode {
        READ, WRITE
    };
    Queue(Mode m, string ifn, int ifp, int qlen, int maxthresh, int max_swr, int max_rwr)
        : mode(m), state(s_init),
        pending_enqueue(0),
        pending_rdma_write(0),
        confirmed_rdma_write(0),
        if_name(ifn),
        if_port(ifp),
        max_send_wr(max_swr),
        max_recv_wr(max_rwr),
        num_send_wr(0),
        crq(max_rwr*sizeof(Message)),
        cwq(max_swr*sizeof(Message)),
        cq_size(max_rwr),
        crq_pending(0),
        ctx(0), pd(0), cc(0), cq(0), qp(0), mbs_mr(0), crq_mr(0), cwq_mr(0)
    {
        Initialize();

        crq_mr = ibv_reg_mr(pd, crq.base, crq.length, IBV_ACCESS_LOCAL_WRITE);
        lcrq_key = crq_mr->lkey;
        cwq_mr = ibv_reg_mr(pd, cwq.base, cwq.length, 0);
        lcwq_key = cwq_mr->lkey;

        mbs.reset(new MirrorBufferSet(qlen, maxthresh));
        lqueue.length = mbs->BufferSize();
        lqueue.base = (char*)(void*)*mbs;
        max_threshold = mbs->MirrorSize();
        mbs_mr = ibv_reg_mr(pd, lqueue.base, lqueue.length + max_threshold, IBV_ACCESS_DEFAULT);
        lqkey = mbs_mr->lkey;


        event_thread.reset(CreatePthreadFunctional(this, &Queue::EntryPoint));

    }

    ~Queue() {
        state = s_stopped;
        Cleanup();
        event_thread->Join();
    }

    void Start() {
        event_thread->Start();
    }

    char *GetEnqueuePtr(uint64_t thresh) {
        AutoLock<PthreadMutex> al(lock);
        assert(thresh <= max_threshold);
        CheckState();
        while (lqueue.Freespace() < thresh) {
            cond.Wait(lock);
        }
        return lqueue.GetHead();
    }

    void Enqueue(uint64_t count) {
        AutoLock<PthreadMutex> al(lock);
        assert(count <= max_threshold);
        assert(count <= lqueue.Freespace());
        CheckState();
        pending_enqueue += count;
        lqueue.AdvanceHead(count);
        CheckRDMAWrite();
    }

    char *GetDequeuePtr(uint64_t thresh) {
        AutoLock<PthreadMutex> al(lock);
        assert(thresh <= max_threshold);
        CheckState();
        while (lqueue.Count() < thresh) {
            cond.Wait(lock);
        }
        return lqueue.GetTail();
    }

    void Dequeue(uint64_t count) {
        AutoLock<PthreadMutex> al(lock);
        assert(count <= max_threshold);
        assert(count <= lqueue.Count());
        CheckState();
        lqueue.AdvanceTail(count);
        PostDequeue(count);
    }

    void Connect(SocketHandle &sock) {
 
        exchange_data(sock, (char*)&local_info, (char*)&remote_info, sizeof(PortInfo));
        ibv_qp_attr attr;
        memset(&attr, 0, sizeof(attr));
        attr.qp_state = IBV_QPS_INIT;
        attr.pkey_index = 0;
        attr.port_num = if_port;
        attr.qp_access_flags = IBV_ACCESS_DEFAULT;
        if (ibv_modify_qp(qp, &attr,
                    IBV_QP_STATE | IBV_QP_ACCESS_FLAGS |
                    IBV_QP_PORT | IBV_QP_PKEY_INDEX))
        {
            throw ErrnoException();
        }
        //PrintQPState();
        PostReceives();
        memset(&attr, 0, sizeof(attr));
        attr.qp_state = IBV_QPS_RTR;
        attr.path_mtu = IBV_MTU_512;

        attr.ah_attr.is_global = 0;
        attr.ah_attr.dlid = remote_info.lid;
        attr.ah_attr.sl = 0;
        attr.ah_attr.src_path_bits = 0;
        attr.ah_attr.port_num = if_port;

        attr.dest_qp_num = remote_info.qp_num;
        attr.rq_psn = 0;
        attr.max_dest_rd_atomic = 1;
        attr.min_rnr_timer = 12; // Recommended value from documentation

        if (ibv_modify_qp(qp, &attr,
                    IBV_QP_STATE | IBV_QP_PATH_MTU |
                    IBV_QP_AV | IBV_QP_DEST_QPN | IBV_QP_RQ_PSN |
                    IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER
                    ))
        {
            throw ErrnoException();
        }
        //PrintQPState();
        memset(&attr, 0, sizeof(attr));
        attr.qp_state = IBV_QPS_RTS;
        attr.timeout = 14; // Recommended from docs
        attr.retry_cnt = 7; // Recommended from docs
        attr.rnr_retry = 7; // Recommended from docs
        attr.sq_psn = 0;
        attr.max_rd_atomic = 1;
        if (ibv_modify_qp(qp, &attr,
                    IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT |
                    IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC
                    ))
        {
            throw ErrnoException();
        }
        //PrintQPState();
        ibv_req_notify_cq(cq, 0);

        if (mode == READ) {
            PostUpdateQueue();
            state = s_running;
        }
    }

private:

    void PrintDeviceAttr(ibv_device_attr &device_attr) {
#define ATTRPRINT(st, name) << #name ": " << (uint64_t)st.name
        cerr << "Device attributes: "
            << hex
            ATTRPRINT(device_attr, node_guid) << ", "
            ATTRPRINT(device_attr, sys_image_guid) << ", "
            << dec
            ATTRPRINT(device_attr, max_mr_size) << ", "
            ATTRPRINT(device_attr, page_size_cap) << ", "
            ATTRPRINT(device_attr, vendor_id) << ", "
            ATTRPRINT(device_attr, vendor_part_id) << ", "
            ATTRPRINT(device_attr, hw_ver) << ", "
            ATTRPRINT(device_attr, max_qp) << ", "
            ATTRPRINT(device_attr, max_qp_wr) << ", "
            << hex ATTRPRINT(device_attr, device_cap_flags) << ", " << dec
            ATTRPRINT(device_attr, max_sge) << ", "
            ATTRPRINT(device_attr, max_sge_rd) << ", "
            ATTRPRINT(device_attr, max_cq) << ", "
            ATTRPRINT(device_attr, max_cqe) << ", "
            ATTRPRINT(device_attr, max_mr) << ", "
            ATTRPRINT(device_attr, max_pd) << ", "
            ATTRPRINT(device_attr, max_qp_rd_atom) << ", "
            ATTRPRINT(device_attr, max_ee_rd_atom) << ", "
            ATTRPRINT(device_attr, max_res_rd_atom) << ", "
            ATTRPRINT(device_attr, max_qp_init_rd_atom) << ", "
            ATTRPRINT(device_attr, max_ee_init_rd_atom) << ", "
            ATTRPRINT(device_attr, atomic_cap) << ", "
            ATTRPRINT(device_attr, max_ee) << ", "
            ATTRPRINT(device_attr, max_rdd) << ", "
            ATTRPRINT(device_attr, max_mw) << ", "
            ATTRPRINT(device_attr, max_raw_ipv6_qp) << ", "
            ATTRPRINT(device_attr, max_raw_ethy_qp) << ", "
            ATTRPRINT(device_attr, max_mcast_grp) << ", "
            ATTRPRINT(device_attr, max_mcast_qp_attach) << ", "
            ATTRPRINT(device_attr, max_total_mcast_qp_attach) << ", "
            ATTRPRINT(device_attr, max_ah) << ", "
            ATTRPRINT(device_attr, max_fmr) << ", "
            ATTRPRINT(device_attr, max_map_per_fmr) << ", "
            ATTRPRINT(device_attr, max_srq) << ", "
            ATTRPRINT(device_attr, max_srq_wr) << ", "
            ATTRPRINT(device_attr, max_srq_sge) << ", "
            ATTRPRINT(device_attr, max_pkeys) << ", "
            ATTRPRINT(device_attr, local_ca_ack_delay) << ", "
            ATTRPRINT(device_attr, phys_port_cnt)
            << endl;
    }

    void PrintPortAttr(ibv_port_attr &port_attr) {
        cerr << "Port " << if_port << " Attributes: "
            << "state: " << ibv_port_state_str(port_attr.state) << ", "
            << hex
            ATTRPRINT(port_attr, max_mtu) << ", "
            ATTRPRINT(port_attr, active_mtu) << ", "
            << dec
            ATTRPRINT(port_attr, gid_tbl_len) << ", "
            << hex
            ATTRPRINT(port_attr, port_cap_flags) << ", "
            << dec
            ATTRPRINT(port_attr, max_msg_sz) << ", "
            ATTRPRINT(port_attr, bad_pkey_cntr) << ", "
            ATTRPRINT(port_attr, qkey_viol_cntr) << ", "
            ATTRPRINT(port_attr, pkey_tbl_len) << ", "
            ATTRPRINT(port_attr, lid) << ", "
            ATTRPRINT(port_attr, sm_lid) << ", "
            ATTRPRINT(port_attr, lmc) << ", "
            ATTRPRINT(port_attr, max_vl_num) << ", "
            ATTRPRINT(port_attr, sm_sl) << ", "
            ATTRPRINT(port_attr, subnet_timeout) << ", "
            ATTRPRINT(port_attr, init_type_reply) << ", "
            ATTRPRINT(port_attr, active_width) << ", "
            ATTRPRINT(port_attr, active_speed) << ", "
            ATTRPRINT(port_attr, phys_state)
            << endl;
    }

    void PrintQPState() {
        ibv_qp_init_attr qp_attr;
        memset(&qp_attr, 0, sizeof(qp_attr));
        ibv_qp_attr attr;
        memset(&attr, 0, sizeof(attr));
        if (ibv_query_qp(qp, &attr, 0, &qp_attr)) {
            throw ErrnoException();
        }
        cerr << "QP State: "
            ATTRPRINT(qp_attr.cap, max_send_wr) << ", "
            ATTRPRINT(qp_attr.cap, max_recv_wr) << ", "
            ATTRPRINT(qp_attr.cap, max_send_sge) << ", "
            ATTRPRINT(qp_attr.cap, max_recv_sge) << ", "
            ATTRPRINT(qp_attr.cap, max_inline_data) << ", "
            << "qp_type: " << (int)qp_attr.qp_type << ", "
            ATTRPRINT(attr, qp_state) << ", "
            ATTRPRINT(attr, cur_qp_state) << ", "
            ATTRPRINT(attr.cap, max_send_wr) << ", "
            ATTRPRINT(attr.cap, max_recv_wr) << ", "
            ATTRPRINT(attr.cap, max_send_sge) << ", "
            ATTRPRINT(attr.cap, max_recv_sge)
            << endl;
    }

    void Initialize() {
        int num_devices = 0;
        ibv_device **devices = ibv_get_device_list(&num_devices);
        ibv_device *device = 0;
        if (num_devices < 1) {
            throw std::runtime_error("Error getting devices, no devices");
        }
        for (int i = 0; i < num_devices; ++i) {
            if (if_name.empty()) {
                device = devices[i];
                break;
            } else if (if_name == ibv_get_device_name(devices[i])) {
                device = devices[i];
                break;
            }
        }
        if (!device) {
            throw std::runtime_error("Error getting device, no matching device.");
        }
        ctx = ibv_open_device(device);
        if (!ctx) { throw ErrnoException(); }
        pd = ibv_alloc_pd(ctx);
        if (!pd) { throw ErrnoException(); }
        cc = ibv_create_comp_channel(ctx);
        if (!cc) { throw ErrnoException(); }
        cq = ibv_create_cq(ctx, max_send_wr + max_recv_wr, 0, cc, 0);
        if (!cq) { throw ErrnoException(); }
        ibv_device_attr device_attr;
        memset(&device_attr, 0, sizeof(device_attr));
        if (ibv_query_device(ctx, &device_attr)) { throw ErrnoException(); }
        //PrintDeviceAttr(device_attr);
        ibv_qp_init_attr qp_attr;
        memset(&qp_attr, 0, sizeof(qp_attr));
        qp_attr.qp_context = 0;
        qp_attr.send_cq = cq;
        qp_attr.recv_cq = cq;
        qp_attr.sq_sig_all = 1;
        qp_attr.cap.max_send_wr = max_send_wr;
        qp_attr.cap.max_recv_wr = max_recv_wr;
        qp_attr.cap.max_send_sge = 1;
        qp_attr.cap.max_recv_sge = 1;
        qp_attr.qp_type = IBV_QPT_RC;
        qp = ibv_create_qp(pd, &qp_attr);
        if (!qp) { throw ErrnoException(); }
        //PrintQPState();
        local_info.qp_num = qp->qp_num;
        ibv_port_attr port_attr;
        memset(&port_attr, 0, sizeof(port_attr));
        if (if_port <= 0) {
            int i;
            for (i = 1; i <= device_attr.phys_port_cnt; ++i) {
                if (ibv_query_port(ctx, i, &port_attr)) {
                    throw ErrnoException();
                }
                if (port_attr.state == IBV_PORT_ACTIVE) {
                    if_port = i;
                    local_info.lid = port_attr.lid;
                    break;
                }
            }
            if (i > device_attr.phys_port_cnt) {
                throw std::runtime_error("Unable to find an active port");
            }
        } else {
            if (ibv_query_port(ctx, if_port, &port_attr)) {
                throw ErrnoException();
            }
            if (port_attr.state == IBV_PORT_ACTIVE) {
                local_info.lid = port_attr.lid;
            } else {
                throw std::runtime_error("Port is not active");
            }
        }
        //PrintPortAttr(port_attr);
    }

    void Cleanup() {
        if (qp) {
            ibv_destroy_qp(qp);
            qp = 0;
        }
        if (cq) {
            ibv_destroy_cq(cq);
            cq = 0;
        }
        if (cc) {
            ibv_destroy_comp_channel(cc);
            cc = 0;
        }
        if (pd) {
            ibv_dealloc_pd(pd);
            pd = 0;
        }
        if (ctx) {
            ibv_close_device(ctx);
            ctx = 0;
        }
    }

    void CheckRDMAWrite() {
        while (true) {
            if (pending_enqueue == 0) return;
            if (num_send_wr >= max_send_wr) return;
            uint64_t r_freespace = rqueue.Freespace() - pending_rdma_write;
            uint64_t count = std::min(r_freespace, pending_enqueue);
            count = std::min(count, max_threshold);
            if (count == 0) return;

            ibv_send_wr wr, *bad;
            ibv_sge sge;
            memset(&sge, 0, sizeof(sge));
            sge.addr = (uint64_t)lqueue.GetTail(pending_rdma_write);
            sge.length = count;
            sge.lkey = lqkey;
            memset(&wr, 0, sizeof(wr));
            wr.wr_id = count;
            wr.sg_list = &sge;
            wr.num_sge = 1;
            wr.opcode = IBV_WR_RDMA_WRITE;
            wr.wr.rdma.remote_addr = (uint64_t)rqueue.GetHead(pending_rdma_write);
            wr.wr.rdma.rkey = rqkey;
            if (ibv_post_send(qp, &wr, &bad)) {
                throw ErrnoException();
            }
            ++num_send_wr;
            pending_enqueue -= count;
            pending_rdma_write += count;
        }
    }

    void PostDequeue(uint64_t count) {
        Message *msg = NewMessage();
        msg->type = mt_dequeue;
        msg->data.dequeue.amount = count;
        PostMessage();
    }

    void PostUpdateQueue() {
        Message *msg = NewMessage();
        msg->type = mt_update_queue;
        msg->data.update_queue.length = lqueue.length;
        msg->data.update_queue.head = lqueue.head;
        msg->data.update_queue.tail = lqueue.tail;
        msg->data.update_queue.rqkey = lqkey;
        msg->data.update_queue.base = lqueue.base;
        PostMessage();
    }

    void PostSend(uint64_t id, void *data, unsigned length, uint32_t lkey) {
        ibv_send_wr wr, *bad;
        ibv_sge sge;
        memset(&sge, 0, sizeof(sge));
        sge.addr = (uint64_t)data;
        sge.length = length;
        sge.lkey = lkey;
        memset(&wr, 0, sizeof(wr));
        wr.wr_id = id;
        wr.sg_list = &sge;
        wr.num_sge = 1;
        wr.opcode = IBV_WR_SEND;
        if (ibv_post_send(qp, &wr, &bad)) {
            throw ErrnoException();
        }
        ++num_send_wr;
    }

    void PostReceives() {
        /*
        for (;crq_pending*sizeof(Message) < crq.Freespace(); ++crq_pending) {
            PostRecv(0, crq.GetHead(crq_pending*sizeof(Message)), sizeof(Message), lcrq_key);
        }
        */
        int num = crq.Freespace()/sizeof(Message) - crq_pending;
        if (num <= 0) return;
        vector<ibv_recv_wr> wr(num);
        memset(&wr[0], 0, num*sizeof(ibv_recv_wr));
        vector<ibv_sge> sge(num);
        memset(&sge[0], 0, num*sizeof(ibv_sge));
        for (int i = 0; i < num; ++i) {
            sge[i].addr = (uint64_t)crq.GetHead(crq_pending*sizeof(Message));
            sge[i].length = sizeof(Message);
            sge[i].lkey = lcrq_key;
            wr[i].sg_list = &sge[i];
            wr[i].num_sge = 1;
            wr[i].next = (i + 1 < num ? &wr[i+1] : 0);
            crq_pending++;
        }
        ibv_recv_wr *bad = 0;
        if (ibv_post_recv(qp, &wr[0], &bad)) {
            int num_left = bad - &wr[0];
            if (errno || num_left == 0) {
                throw ErrnoException();
            } else {
                crq_pending -= num_left;
            }
        }
    }

    void PostRecv(uint64_t id, void *data, unsigned length, uint32_t lkey) {
        ibv_recv_wr wr, *bad;
        memset(&wr, 0, sizeof(wr));
        ibv_sge sge;
        memset(&sge, 0, sizeof(sge));
        sge.addr = (uint64_t)data;
        sge.length = length;
        sge.lkey = lkey;
        wr.wr_id = id;
        wr.sg_list = &sge;
        wr.num_sge = 1;
        if (ibv_post_recv(qp, &wr, &bad)) {
            throw ErrnoException();
        }
    }
    void *EntryPoint() {
        vector<ibv_wc> wcs(100);
        while (state != s_stopped && state != s_error) {
            fd_set fdsr;
            int maxfd = max(cc->fd, ctx->async_fd) + 1;
            FD_ZERO(&fdsr);
            FD_SET(cc->fd, &fdsr);
            FD_SET(ctx->async_fd, &fdsr);
            select(maxfd, &fdsr, 0, 0, 0);
            if (FD_ISSET(cc->fd, &fdsr)) {
                ibv_cq *l_cq;
                void *context;
                if (ibv_get_cq_event(cc, &l_cq, &context)) {
                    state = s_error;
                    break;
                }
                ibv_req_notify_cq(cq, 0);
                while (true) {
                    int num = ibv_poll_cq(cq, wcs.size(), &wcs[0]);
                    for (int i = 0; i < num; ++i) {
                        ibv_wc *wc = &wcs[i];
                        if (wc->status != IBV_WC_SUCCESS) {
                            state = s_error;
                            break;
                        }
                        HandleWorkCompletion(wc);
                    }
                    ProcessEvents();
                    if (num < (int)wcs.size()) break;
                }
                ibv_ack_cq_events(l_cq, 1);
            }
            if (FD_ISSET(ctx->async_fd, &fdsr)) {
                ibv_async_event event;
                memset(&event, 0, sizeof(event));
                if (!ibv_get_async_event(ctx, &event)) {
                    cerr << "Received Async event \""
                        << ibv_event_type_str(event.event_type)
                        << "\"\n";
                    ibv_ack_async_event(&event);
                }
            }
        }
        return 0;
    }

    void HandleWorkCompletion(ibv_wc *wc) {
        AutoLock<PthreadMutex> al(lock);
        switch (wc->opcode) {
        case IBV_WC_SEND:
            cwq.AdvanceTail(sizeof(Message));
            --num_send_wr;
            cond.Signal();
            break;
        case IBV_WC_RDMA_WRITE:
            {
                uint64_t count = wc->wr_id;
                lqueue.AdvanceTail(count);
                rqueue.AdvanceHead(count);
                pending_rdma_write -= count;
                confirmed_rdma_write += count;
                --num_send_wr;
                cond.Signal();
            }
            break;
        case IBV_WC_RECV:
            crq.AdvanceHead(sizeof(Message));
            crq_pending--;
            break;
        default:
            assert(false);
            break;
        }
    }

    void ProcessEvents() {
        AutoLock<PthreadMutex> al(lock);
        while (!crq.Empty()) {
            ProcessMessage((Message*)crq.GetTail());
            crq.AdvanceTail(sizeof(Message));
        }
        if (mode == WRITE) {
            CheckRDMAWrite();
            if (confirmed_rdma_write > 0 && !cwq.Full() && num_send_wr < max_send_wr) {
                Message *msg = NewMessage();
                msg->type = mt_enqueue;
                msg->data.enqueue.amount = confirmed_rdma_write;
                PostMessage();
                confirmed_rdma_write = 0;
            }
        }
        PostReceives();
    }

    void ProcessMessage(Message *msg) {
        switch (msg->type) {
        case mt_update_queue:
            if (state != s_init) break;
            rqueue.length = msg->data.update_queue.length;
            rqueue.head = msg->data.update_queue.head;
            rqueue.tail = msg->data.update_queue.tail;
            rqkey = msg->data.update_queue.rqkey;
            rqueue.base = msg->data.update_queue.base;
            state = s_running;
            cond.Signal();
            break;
        case mt_dequeue:
            if (state != s_running) break;
            rqueue.AdvanceTail(msg->data.dequeue.amount);
            break;
        case mt_enqueue:
            if (state != s_running) break;
            lqueue.AdvanceHead(msg->data.enqueue.amount);
            cond.Signal();
            break;
        }
    }

    void CheckState() {
        if (state == s_error) {
            throw std::runtime_error("An error occured in the queue");
        }
        while (true) {
            if (state == s_running) break;
            if (state == s_stopped) break;
            cond.Wait(lock);
        }
    }

    Message *NewMessage() {
        while (cwq.Full() || num_send_wr >= max_send_wr) {
            cond.Wait(lock);
        }
        Message *msg = (Message*)cwq.GetHead();
        memset(msg, 0, sizeof(Message));
        return msg;
    }

    void PostMessage() {
        char *ptr = cwq.GetHead();
        PostSend(0, ptr, sizeof(Message), lcwq_key);
        cwq.AdvanceHead(sizeof(Message));
    }

    Mode mode;
    State state;
    auto_ptr<Pthread> event_thread;
    auto_ptr<MirrorBufferSet> mbs;
    QueueModel lqueue;
    QueueModel rqueue;
    uint32_t lqkey;
    uint32_t rqkey;
    uint64_t pending_enqueue;
    uint64_t pending_rdma_write;
    uint64_t confirmed_rdma_write;
    uint64_t max_threshold;
    PthreadMutex lock;
    PthreadCondition cond;
    string if_name;
    int if_port;
    uint32_t max_send_wr, max_recv_wr;
    uint32_t num_send_wr;
    PortInfo local_info;
    PortInfo remote_info;

    MemQueue crq;
    MemQueue cwq;
    uint32_t lcrq_key;
    uint32_t lcwq_key;
    int cq_size;
    int crq_pending;


    ibv_context *ctx;
    ibv_pd *pd;
    ibv_comp_channel *cc;
    ibv_cq *cq;
    ibv_qp *qp;
    ibv_mr *mbs_mr;
    ibv_mr *crq_mr;
    ibv_mr *cwq_mr;
};

int main(int argc, char **argv) {
    string inet_host = "0.0.0.0";
    string inet_port = "12345";
    string if_name;
    int if_port = -1;
    bool server = false;
    uint64_t queue_len = 1000000;
    uint64_t max_thresh = 100000;
    double rate_throttle = -1;
    double report_time = 0.1;
    bool verify = false;

    while (true) {
        static option longopts[] = {
            { "help",       false,  0,  'h'},
            { "inet-host",  true,   0,  'H'},
            { "inet-port",  true,   0,  'p'},
            { "if-name",    true,   0,  'n'},
            { "if-port",    true,   0,  'P'},
            { "server",     false,  0,  's'},
            { "queue",      true,   0,  'q'},
            { "threshold",  true,   0,  't'},
            { "max-rate",   true,   0,  'm'},
            { "report-rate",true,   0,  'r'},
            { "verify",     false,  0,  'v'},
            { 0, 0, 0, 0}
        };
        int c = getopt_long(argc, argv, "H:p:n:P:shq:t:m:r:v", &longopts[0], 0);
        if (c == -1) break;
        switch (c) {
        case 'H':
            inet_host = optarg;
            break;
        case 'p':
            inet_port = optarg;
            break;
        case 'n':
            if_name = optarg;
            break;
        case 'P':
            if_port = atoi(optarg);
            break;
        case 's':
            server = true;
            break;
        case 'q':
            queue_len = atoi(optarg);
            break;
        case 't':
            max_thresh = atoi(optarg);
            break;
        case 'm':
            rate_throttle = strtod(optarg, 0);
            break;
        case 'r':
            report_time = strtod(optarg, 0);
            break;
        case 'v':
            verify = true;
            break;
        case 'h':
        default:
            cerr << "Usage: " << *argv << " [options]\n"
                << "\t--inet-host nnnn\n"
                << "\t-H nnnn\tSpecify the network hostname to use.\n"
                << "\t--inet-port nnnn\n"
                << "\t-p nnnn\tSpecify the network port to use.\n"
                << "\t--if-name nnnn\n"
                << "\t-n nnnn\tSpecify the infiniband device name to use (default: first one found)\n"
                << "\t--if-port nnnn\n"
                << "\t-P nnnn\tSpecify the physical port on the infiniband card to use (default: first with status ACTIVE)\n"
                << "\t--server\n"
                << "\t-s     \tSpecify that this should act as a server rather than client\n"
                << "\t--help\n"
                << "\t-h     \tPrint this message and exit\n"
                << "\t--queue nnn\n"
                << "\t-q nnn \tSet the queue size\n"
                << "\t--threshold nnn\n"
                << "\t-t nnn \tSet the threshold size\n"
                << "\t--max-rate nnn\n"
                << "\t-m nnn \tSet the maximum rate to transfer at in bytes per per second\n"
                << "\t--report-rate nnn\n"
                << "\t-r nnn \tSet the number of seconds between reporting\n"
                ;
            return 0;
            break;
        }
    }
    if (max_thresh % sizeof(uint64_t) > 0) {
        max_thresh += sizeof(uint64_t) - (max_thresh % sizeof(uint64_t));
    }

    uint64_t total_bytes = 0;
    double total_time = 0;
    double min_rate = std::numeric_limits<double>::infinity();
    double max_rate = 0;
    int max_send_wr = 10;
    int max_recv_wr = 10;
    uint64_t counter = 0;
    std::vector<double> filt_buf(filt_len, 0);
    if (server) {
        ServerSocketHandle serv;
        serv.Listen(SocketAddress::CreateIP(inet_host, inet_port));
        serv.SetReuseAddr();
        SocketAddress addr;
        addr.SetFromSockName(serv.FD());
        cout << "Listening on " << addr.GetHostName() << " " << addr.GetServName() << endl;
        SocketHandle sock(serv.Accept());
        serv.Close();
        cout << "Connection received, receiving" << endl;
        Queue queue(Queue::READ, if_name, if_port, queue_len, max_thresh, max_send_wr, max_recv_wr);
        queue.Connect(sock);
        sock.Close();
        queue.Start();
        queue.GetDequeuePtr(max_thresh);
        while (true) {
            double start = getTime();
            uint64_t numread = 0;
            while (getTime() - start < report_time) {
                const uint64_t *ptr = (const uint64_t*)queue.GetDequeuePtr(max_thresh);
                if (verify) {
                    for (unsigned i = 0; i < max_thresh/sizeof(uint64_t); ++i, ++counter) {
                        if (ptr[i] != counter) {
                            cerr << "Error on " << ptr[i] << " != " << counter << endl;
                            throw counter;
                        }
                    }
                }
                queue.Dequeue(max_thresh);
                numread += max_thresh;
            }
            double end = getTime();
            double dur = end - start;
            total_bytes += numread;
            total_time += dur;
            double rate = numread/dur;
            min_rate = min(rate, min_rate);
            max_rate = max(rate, max_rate);
            double lp_rate = Filter(rate, filt_b, filt_len, filt_a, filt_len, &filt_buf[0]);
            cout << "Recv "
                << Bytes(numread) << " in " << Time(dur) << " at " << Bytes(rate)
                << "/sec;\n" << "Min: " << Bytes(min_rate) << "/s Max: " << Bytes(max_rate)
                << "/s Avg: " << Bytes(total_bytes/total_time) << "/s"
                << " LP: " << Bytes(lp_rate) << "/s"
                << endl;
        }
    } else {
        cout << "Connecting" << endl;
        SocketHandle sock;
        sock.Connect(SocketAddress::CreateIP(inet_host, inet_port));
        Queue queue(Queue::WRITE, if_name, if_port, queue_len, max_thresh, max_send_wr, max_recv_wr);
        queue.Connect(sock);
        sock.Close();
        queue.Start();
        while (true) {
            double start = getTime();
            double cur = getTime();
            uint64_t numsent = 0;
            while (cur - start < report_time) {
                if (rate_throttle > 0 && numsent > 0) {
                    double min_time = numsent/rate_throttle;
                    double dur = cur - start;
                    if (min_time > dur) {
                        Sleep(min_time - dur);
                    }
                }
                uint64_t *ptr = (uint64_t*)queue.GetEnqueuePtr(max_thresh);
                if (verify) {
                    for (unsigned i = 0; i < max_thresh/sizeof(uint64_t); ++i, ++counter) {
                        ptr[i] = counter;
                    }
                }
                queue.Enqueue(max_thresh);
                numsent += max_thresh;
                cur = getTime();
            }
            double end = cur;
            double dur = end - start;
            total_bytes += numsent;
            total_time += dur;
            double rate = numsent/dur;
            min_rate = min(rate, min_rate);
            max_rate = max(rate, max_rate);
            double lp_rate = Filter(rate, filt_b, filt_len, filt_a, filt_len, &filt_buf[0]);
            cout << "Sent "
                << Bytes(numsent) << " in " << Time(dur) << " at " << Bytes(rate)
                << "/sec;\n" << "Min: " << Bytes(min_rate) << "/s Max: " << Bytes(max_rate)
                << "/s Avg: " << Bytes(total_bytes/total_time) << "/s"
                << " LP: " << Bytes(lp_rate) << "/s"
                << endl;
        }
    }
    return 0;
}

