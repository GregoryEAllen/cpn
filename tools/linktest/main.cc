
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <limits>
#include <iomanip>
#include <cmath>
#include <errno.h>
#include "SocketHandle.h"
#include "ServerSocketHandle.h"
#include "ErrnoException.h"

using namespace std;

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


int main(int argc, char **argv) {
    bool server = false;
    std::string hostname;
    std::string portname;
    int buffer_len = 8192;
    double report_time = 0.1;
    double rate_throttle = -1;
    while (true) {
        int c = getopt(argc, argv, "sh:p:b:r:m:");
        if (c == -1) break;
        switch(c) {
        case 's':
            server = true;
            break;
        case 'h':
            hostname = optarg;
            break;
        case 'p':
            portname = optarg;
            break;
        case 'b':
            buffer_len = atoi(optarg);
            break;
        case 'r':
            report_time = strtod(optarg, 0);
            break;
        case 'm':
            rate_throttle = strtod(optarg, 0);
            break;
        default:
            break;
        }
    }
    SockAddrList addrs = SocketAddress::CreateIP(hostname, portname);

    uint64_t total_bytes = 0;
    double total_time = 0;
    double min_rate = std::numeric_limits<double>::infinity();
    double max_rate = 0;
    vector<char> buffer(buffer_len, 0);
    cout.width(3);
    cout.precision(3);
    cout.setf(ios::fixed);
    cout << setfill(' ');
    if (server) {
        ServerSocketHandle sockserv(addrs);
        sockserv.SetReuseAddr();
        SocketAddress addr;
        addr.SetFromSockName(sockserv.FD());
        cout << "Listening on " << addr.GetHostName() << " " << addr.GetServName() << endl;
        SocketHandle sock(sockserv.Accept());
        sockserv.Close();
        cout << "Connection received, receiving" << endl;
        sock.Read(&buffer[0], buffer.size());
        while (true) {
            double start = getTime();
            uint64_t numread = 0;
            while (getTime() - start < report_time) {
                numread += sock.Read(&buffer[0], buffer.size());
            }
            if (numread == 0) {
                if (sock.Eof()) break;
            }
            double end = getTime();
            double dur = end - start;
            total_bytes += numread;
            total_time += dur;
            double rate = numread/dur;
            min_rate = min(rate, min_rate);
            max_rate = max(rate, max_rate);
            cout << "Recv "
                << Bytes(numread) << " in " << Time(dur) << " at " << Bytes(rate)
                << "/sec;\n " << " Min: " << Bytes(min_rate) << "/s Max: " << Bytes(max_rate)
                << "/s Avg: " << Bytes(total_bytes/total_time) << "/s" << endl;
        }
    } else {
        cout << "Connecting" << endl;
        SocketHandle sock;
        sock.Connect(addrs);
        sock.Write(&buffer[0], buffer.size());
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
                numsent += sock.Write(&buffer[0], buffer.size());
                cur = getTime();
            }
            double end = cur;
            double dur = end - start;
            total_bytes += numsent;
            total_time += dur;
            double rate = numsent/dur;
            min_rate = min(rate, min_rate);
            max_rate = max(rate, max_rate);
            cout << "Sent "
                << Bytes(numsent) << " in " << Time(dur) << " at " << Bytes(rate)
                << "/sec;\n " << " Min: " << Bytes(min_rate) << "/s Max: " << Bytes(max_rate)
                << "/s Avg: " << Bytes(total_bytes/total_time) << "/s" << endl;
        }
    }
    return 0;
}
