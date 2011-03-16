
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <iostream>
#include <iomanip>
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


int main(int argc, char **argv) {
    bool server = false;
    std::string hostname;
    std::string portname;
    int buffer_len = 8192;
    double report_time = 0.1;
    while (true) {
        int c = getopt(argc, argv, "sh:p:b:r:");
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
        default:
            break;
        }
    }
    SockAddrList addrs = SocketAddress::CreateIP(hostname, portname);

    uint64_t total_bytes = 0;
    double total_time = 0;
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
            cout << "Received " << Bytes(numread) << " in " << Time(dur) << " at " <<
                Bytes(numread/dur) << "/sec; Avg: " << Bytes(total_bytes/total_time) << "/sec" << endl;
        }
    } else {
        cout << "Connecting" << endl;
        SocketHandle sock;
        sock.Connect(addrs);
        sock.Write(&buffer[0], buffer.size());
        while (true) {
            double start = getTime();
            uint64_t numsent = 0;
            while (getTime() - start < report_time) {
                numsent += sock.Write(&buffer[0], buffer.size());
            }
            double end = getTime();
            double dur = end - start;
            total_bytes += numsent;
            total_time += dur;
            cout << "Sent " << Bytes(numsent) << " in " << Time(dur) << " at " <<
                Bytes(numsent/dur) << "/sec; Avg: " << Bytes(total_bytes/total_time) << "/sec" << endl;
        }
    }
    return 0;
}
