
#include <stdlib.h>
#include <sys/time.h>
#include <iostream>
#include <string>
#include <iomanip>
#include "Kernel.h"
#include "NodeBase.h"
#include "ErrnoException.h"
#include "IQueue.h"
#include "OQueue.h"
#include "RemoteContext.h"
#include "SocketAddress.h"

using namespace std;
using namespace CPN;

static double getTime() {
    timeval tv;
    if (gettimeofday(&tv, 0) != 0) {
        throw ErrnoException();
    }
    return static_cast<double>(tv.tv_sec) + 1e-6 * static_cast<double>(tv.tv_usec);
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

void Sender(NodeBase *node, int buffer_len, double report_time) {
    cout << "Sender start" << endl;
    OQueue<char> out = node->GetOQueue("out");
    uint64_t total_bytes = 0;
    double total_time = 0;
    while (true) {
        double start = getTime();
        uint64_t numsent = 0;
        while (getTime() - start < report_time) {
            out.GetEnqueuePtr(buffer_len);
            out.Enqueue(buffer_len);
            numsent += buffer_len;
        }
        double end = getTime();
        double dur = end - start;
        total_bytes += numsent;
        total_time += dur;
        cout << "Sent " << Bytes(numsent) << " in " << Time(dur) << " at " <<
            Bytes(numsent/dur) << "/sec; Avg: " << Bytes(total_bytes/total_time) << "/sec" << endl;
    }
}

void Receiver(NodeBase *node, int buffer_len, double report_time) {
    cout << "Receiver start" << endl;
    IQueue<char> in = node->GetIQueue("in");
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
        cout << "Recv " << Bytes(numread) << " in " << Time(dur) << " at " <<
            Bytes(numread/dur) << "/sec; Avg: " << Bytes(total_bytes/total_time) << "/sec" << endl;
    }
}

int main(int argc, char **argv) {
    string kernelhost = "localhost";
    string kernelport = "0";
    int buffer_len = 8192;
    bool local = false;
    bool sender = false;
    int queue_size = 0;
    int threshold_size = 0;
    bool threshold = false;
    double report_time = 0.1;
    while (true) {
        int c = getopt(argc, argv, "h:p:sb:q:t:lTr:");
        if (c == -1) break;
        switch (c) {
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
            threshold = true;
            break;
        case 'r':
            report_time = strtod(optarg, 0);
            break;
        default:
            break;
        }
    }

    if (local) {
        Kernel kernel("kernel");
        kernel.CreateFunctionNode("send", Sender, buffer_len, report_time);
        kernel.CreateFunctionNode("recv", Receiver, buffer_len, report_time);
        QueueAttr qattr( (queue_size > 0 ? queue_size : 2*buffer_len), (threshold_size > 0 ? threshold_size : buffer_len) );
        qattr.SetWriter("send", "out").SetReader("recv", "in");
        qattr.SetHint(QUEUEHINT_THRESHOLD);
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
            Kernel kernel(KernelAttr("send").SetContext(context).SetHostName(kernelhost).SetServName(kernelport));
            kernel.CreateFunctionNode("send", Sender, buffer_len, report_time);
            QueueAttr qattr( max(queue_size, 2*buffer_len), max(threshold_size, buffer_len) );
            qattr.SetWriter("send", "out").SetReader("recv", "in");
            qattr.SetHint(QUEUEHINT_THRESHOLD);
            kernel.CreateQueue(qattr);
            kernel.Wait();
        } else {
            Kernel kernel(KernelAttr("recv").SetContext(context).SetHostName(kernelhost).SetServName(kernelport));
            kernel.CreateFunctionNode("recv", Receiver, buffer_len, report_time);
            kernel.Wait();
        }
    }
    cout << "end" << endl;
    return 0;
}
