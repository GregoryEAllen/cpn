
#include <stdlib.h>
#include <sys/time.h>
#include <iostream>
#include <string>
#include <limits>
#include <iomanip>
#include <errno.h>
#include "Kernel.h"
#include "ParseBool.h"
#include "NodeBase.h"
#include "ErrnoException.h"
#include "IQueue.h"
#include "OQueue.h"
#include "RemoteContext.h"
#include "SocketAddress.h"
#include "Filter.h"

using namespace std;
using namespace CPN;

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
    while (v > 1000 && p < 5) {
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

const double a[] = { 1, -1.88903307939452, 0.894874344616636 };
const double b[] = { 0.00146031630552773, 0.00292063261105546, 0.00146031630552773 };
const unsigned filt_len = 3;

void Sender(NodeBase *node, int buffer_len, double report_time, double rate_throttle) {
    cout << "Sender start" << endl;
    std::vector<double> filt_buf(filt_len, 0);
    OQueue<char> out = node->GetOQueue("out");
    double min_rate = std::numeric_limits<double>::infinity();
    double max_rate = 0;
    uint64_t total_bytes = 0;
    double total_time = 0;
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
            out.GetEnqueuePtr(buffer_len);
            out.Enqueue(buffer_len);
            numsent += buffer_len;
            cur = getTime();
        }
        double end = cur;
        double dur = end - start;
        total_bytes += numsent;
        total_time += dur;
        double rate = numsent/dur;
        min_rate = min(rate, min_rate);
        max_rate = max(rate, max_rate);
        double lp_rate = Filter(rate, b, filt_len, a, filt_len, &filt_buf[0]);
        cout << "Sent "
            << Bytes(numsent) << " in " << Time(dur) << " at " << Bytes(rate)
            << "/sec;\n" << "Min: " << Bytes(min_rate) << "/s Max: " << Bytes(max_rate)
            << "/s Avg: " << Bytes(total_bytes/total_time) << "/s"
            << " LP: " << Bytes(lp_rate) << "/s"
            << endl;
    }
}

void Receiver(NodeBase *node, int buffer_len, double report_time) {
    cout << "Receiver start" << endl;
    std::vector<double> filt_buf(filt_len, 0);
    IQueue<char> in = node->GetIQueue("in");
    double min_rate = std::numeric_limits<double>::infinity();
    double max_rate = 0;
    uint64_t total_bytes = 0;
    double total_time = 0;
    while (true) {
        double start = getTime();
        uint64_t numread = 0;
        while (getTime() - start < report_time) {
            if (!in.GetDequeuePtr(buffer_len)) {
                cout << "end of input" << endl;
                return;
            }
            in.Dequeue(buffer_len);
            numread += buffer_len;
        }
        double end = getTime();
        double dur = end - start;
        total_bytes += numread;
        total_time += dur;
        double rate = numread/dur;
        min_rate = min(rate, min_rate);
        max_rate = max(rate, max_rate);
        double lp_rate = Filter(rate, b, filt_len, a, filt_len, &filt_buf[0]);
        cout << "Recv "
            << Bytes(numread) << " in " << Time(dur) << " at " << Bytes(rate)
            << "/sec;\n" << "Min: " << Bytes(min_rate) << "/s Max: " << Bytes(max_rate)
            << "/s Avg: " << Bytes(total_bytes/total_time) << "/s"
            << " LP: " << Bytes(lp_rate) << "/s"
            << endl;
    }
}

int main(int argc, char **argv) {
    string kernelhost = "localhost";
    string kernelport = "";
    int buffer_len = 8192;
    bool local = false;
    bool sender = false;
    int queue_size = 0;
    int threshold_size = 0;
    bool threshold = true;
    double report_time = 0.1;
    double rate_throttle = -1;
    int maxwritethresh = -1;
    double alpha = -1;
    while (true) {
        int c = getopt(argc, argv, "h:p:sb:q:t:lTr:Hm:R:a:");
        if (c == -1) break;
        switch (c) {
        case 'a':
            alpha = strtod(optarg, 0);
            break;
        case 'h':
            kernelhost = optarg;
            break;
        case 'p':
            kernelport = optarg;
            break;
        case 's':
            sender = true;
            break;
        case 'b':
            buffer_len = atoi(optarg);
            break;
        case 'l':
            local = true;
            break;
        case 'q':
            queue_size = atoi(optarg);
            break;
        case 't':
            threshold_size = atoi(optarg);
            break;
        case 'T':
            threshold = ParseBool(optarg);
            break;
        case 'r':
            report_time = strtod(optarg, 0);
            break;
        case 'm':
            rate_throttle = strtod(optarg, 0);
            break;
        case 'R':
            maxwritethresh = atoi(optarg);
            break;
        case 'H':
        default:
            cerr << "Usage: " << *argv << " [options] <context host> <context port>\n"
                "       " << *argv << " -l [options]\n"
                "\t-h n\t kernel hostname\n"
                "\t-p n\t kernel port name\n"
                "\t-s  \t Act as sender\n"
                "\t-b n\t Specify buffer size (current: " << buffer_len << ")\n"
                "\t-l  \t Run locally.\n"
                "\t-q n\t Specify queue size (current: " << queue_size << ")\n"
                "\t-t n\t Specify threshold size (current " << threshold_size << ")\n"
                "\t-Tyn\t Specify whether to use the vmm queue (current: " << threshold << ")\n"
                "\t-r n\t Time in seconds between reports (current: " << report_time << ")\n"
                "\t-R n\t Set max write threshold on the queue (default: 1000000)\n"
                "\t-a n\t Set the alpha parameter (default: 0.5)\n"
                "\t-m n\t Maximum speed in bytes per second to throttle to (<=0 no throttle) (current: "
                << rate_throttle << ")\n"
                ;
            return 0;
        }
    }

    if (local) {
        Kernel kernel("kernel");
        kernel.CreateFunctionNode("send", Sender, buffer_len, report_time, rate_throttle);
        kernel.CreateFunctionNode("recv", Receiver, buffer_len, report_time);
        QueueAttr qattr( (queue_size > 0 ? queue_size : 2*buffer_len), (threshold_size > 0 ? threshold_size : buffer_len) );
        qattr.SetWriter("send", "out").SetReader("recv", "in");
        if (threshold) {
            qattr.SetHint(QUEUEHINT_THRESHOLD);
        }
        kernel.CreateQueue(qattr);
        kernel.Wait();
    } else {
        if (optind+1 > argc) {
            cout << "not enough params " << endl;
            return 1;
        }
        shared_ptr<Context> context;
        context.reset(new RemoteContext(SocketAddress::CreateIP(argv[optind], argv[optind+1])));
        if (sender) {
            Kernel kernel(KernelAttr("send").SetContext(context).SetHostName(kernelhost).SetServName(kernelport).UseD4R(false));
            kernel.CreateFunctionNode("send", Sender, buffer_len, report_time, rate_throttle);
            QueueAttr qattr( max(queue_size, 2*buffer_len), max(threshold_size, buffer_len) );
            qattr.SetWriter("send", "out").SetReader("recv", "in");
            if (threshold) {
                qattr.SetHint(QUEUEHINT_THRESHOLD);
            }
            if (maxwritethresh > 0) {
                qattr.SetMaxWriteThreshold(maxwritethresh);
            }
            if (alpha > 0) {
                qattr.SetAlpha(alpha);
            }
                kernel.CreateQueue(qattr);
            kernel.Wait();
        } else {
            Kernel kernel(KernelAttr("recv").SetContext(context).SetHostName(kernelhost).SetServName(kernelport).UseD4R(false));
            kernel.CreateFunctionNode("recv", Receiver, buffer_len, report_time);
            kernel.Wait();
        }
    }
    cout << "end" << endl;
    return 0;
}
