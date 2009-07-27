/** \file
 */

#include "Socket.h"
#include "AutoCircleBuffer.h"
#include <fcntl.h>
#include <cstdio>

using Socket::SocketAddress;
using Socket::StreamSocket;
using Socket::Poll;
using Socket::PollData;

const int BUFF_SIZE = 256;

struct SockData {
    SockData(StreamSocket* s, bool l) : sock(s), buff(BUFF_SIZE), listen(l) {}
    ~SockData() { StreamSocket::DeleteStreamSocket(sock); }
    StreamSocket* sock;
    AutoCircleBuffer buff;
    bool listen;
};

void SetBlocking(int fd, bool block) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags) { flags = 0; }
    if (!block) {
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    } else {
        fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Error must be: %s port\n", argv[0]);
        return 0;
    }
    std::vector<SocketAddress> addresses;
    if (SocketAddress::Lookup(0, argv[1], Socket::INET, addresses) != 0) {
        printf("Address lookup failed\n");
        return 0;
    }
    StreamSocket* lsock = 0;
    for (std::vector<SocketAddress>::iterator itr = addresses.begin();
            itr != addresses.end(); ++itr) {
        lsock = StreamSocket::NewStreamSocket(itr->Family());
        printf("Attempting bind to %s\n", itr->StringAddress().c_str());
        if (lsock->Bind(*itr)) {
            printf("Bind successful\n");
            if (lsock->Listen(10)) {
                printf("Listen successful\n");
            }
            break;
        }
        printf("Failed to connect.\n");
        StreamSocket::DeleteStreamSocket(lsock);
        lsock = 0;
    }
    if (0 == lsock) {
        return 0;
    }

    std::vector<SockData*> socks;
    socks.push_back(new SockData(lsock, true));
    std::vector<PollData> polldata;
    std::vector<SockData*> keepsocks;
    std::vector<SockData*> deletesocks;
    while (socks.size() > 0) {
        polldata.clear();
        for (std::vector<SockData*>::iterator itr = socks.begin();
                itr != socks.end(); ++itr) {
            polldata.push_back(PollData((*itr)->sock, true, ((*itr)->buff.Size() != 0)));
        }
        int ret = Poll(polldata, -1);

        for (int i = 0; i < polldata.size(); ++i) {
            bool keep = true;
            if (polldata[i].In()) {
                printf("Sock %d data in\n", i);
                if (socks[i]->listen) {
                    StreamSocket *s = socks[i]->sock->Accept(false);
                    keepsocks.push_back(new SockData(s, false));
                    printf("Accepted a connection\n");
                } else {
                    int numtoread = 0;
                    char* in = socks[i]->buff.AllocatePut(BUFF_SIZE, numtoread);
                    int numread = socks[i]->sock->Read(in, numtoread, false);
                    if (numread < 0) {
                        keep = false;
                    } else if (numread == 0 && socks[i]->sock->GetState() == StreamSocket::BAD) {
                        printf("Orderly shutdown.\n");
                        keep = false;
                    } else {
                        printf("Read: ");
                        fwrite(in, numread, 1, stdout);
                        printf("\n");
                        socks[i]->buff.ReleasePut(numread);
                    }
                }
            }
            if (polldata[i].Out()) {
                printf("Sock %d data out\n", i);
                int numtowrite = 0;
                char* out = socks[i]->buff.AllocateGet(socks[i]->buff.Size(), numtowrite);
                int numwritten = socks[i]->sock->Write(out, numtowrite, false);
                if (numwritten < 0) {
                    keep = false;
                } else {
                    printf("Wrote: ");
                    fwrite(out, 1, numwritten, stdout);
                    printf("\n");
                    socks[i]->buff.ReleaseGet(numwritten);
                }
            }
            if (polldata[i].Hup()) {
                printf("Sock %d disconnect\n", i);
                keep = false;
            }
            if (polldata[i].Err()) {
                printf("Sock %d error\n", i);
                keep = false;
            }
            if (keep) {
                keepsocks.push_back(socks[i]);
            } else {
                deletesocks.push_back(socks[i]);
            }
        }
        for (std::vector<SockData*>::iterator itr = deletesocks.begin();
                itr != deletesocks.end(); ++itr) {
            delete *itr;
            printf("Lost a connection\n");
        }
        socks.clear();
        deletesocks.clear();
        socks.swap(keepsocks);
    }
    for (std::vector<SockData*>::iterator itr = keepsocks.begin();
            itr != keepsocks.end(); ++itr) {
        delete *itr;
    }
	return 0;
}




