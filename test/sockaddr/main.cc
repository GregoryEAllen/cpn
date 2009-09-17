
#include "AsyncSocket.h"
#include "Assert.h"
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv) {
    bool procArgs = true;
    char *host = 0;
    char *port = 0;
    while (procArgs) {
        switch (getopt(argc, argv, "h:p:")) {
        case 'h':
            host = optarg;
            break;
        case 'p':
            port = optarg;
            break;
        case -1:
            procArgs = false;
            break;
        default:
            printf("Unknown option");
        }
    }
    Async::SockAddrList list;
    if (host && port) {
        list = Async::SocketAddress::CreateIP(host, port);
    } else if (port) {
        list = Async::SocketAddress::CreateIPFromServ(port);
    } else if (host) {
        list = Async::SocketAddress::CreateIPFromHost(host);
    }
    for (Async::SockAddrList::iterator itr = list.begin();
            itr != list.end(); ++itr) {
        switch (itr->GetType()) {
        case Async::SocketAddress::IPV4:
            printf("IPV4 ");
            break;
        case Async::SocketAddress::IPV6:
            printf("IPV6 ");
            break;
        case Async::SocketAddress::LOCAL:
            printf("LOCAL ");
            break;
        default:
            ASSERT(false);
        }
        printf("%s %s\n", itr->GetHostName().c_str(), itr->GetServName().c_str());
    }
    return 0;
}

