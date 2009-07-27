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
    if (argc != 3) {
        printf("Error must be: %s host port\n", argv[0]);
        return 0;
    }
    std::vector<SocketAddress> addresses;
    if (SocketAddress::Lookup(argv[1], argv[2], Socket::INET, addresses) != 0) {
        printf("Address lookup failed\n");
        return 0;
    }
    StreamSocket* sock = 0;
    for (std::vector<SocketAddress>::iterator itr = addresses.begin();
            itr != addresses.end(); ++itr) {
        sock = StreamSocket::NewStreamSocket(itr->Family());
        printf("Attempting to connect to %s\n", itr->StringAddress().c_str());
        if (sock->Connect(*itr)) {
            printf("Connection successful\n");
            break;
        }
        printf("Failed to connect.\n");
        StreamSocket::DeleteStreamSocket(sock);
        sock = 0;
    }
    if (0 == sock) {
        return 0;
    }

    int tin = fileno(stdin);
    int tout = fileno(stdout);
    AutoCircleBuffer scbin(BUFF_SIZE);
    AutoCircleBuffer scbout(BUFF_SIZE);
    SetBlocking(tin, false);
    // should likely not be used with the standard c functions anymore...
    // and we have to buffer it ourselves
    std::vector<PollData> polldata;
    polldata.push_back(PollData(sock, true, false));
    polldata.push_back(PollData(tout, false, false));
    polldata.push_back(PollData(tin, true, false));
    while (true) {
        polldata[0].Reset(true, (scbin.Size() > 0));
        polldata[1].Reset(false, (scbout.Size() > 0));
        polldata[2].Reset(true, false);
        int ret = Poll(polldata, -1);
        if (polldata[0].In()) {
            int numtoread = 0;
            char* in = scbout.AllocatePut(BUFF_SIZE, numtoread);
            int numread = sock->Read(in, numtoread, false);
            if (numread < 0) { break; }
            printf("Read %d from sock\n", numread);
            scbout.ReleasePut(numread);
        }
        if (polldata[0].Out()) {
            int numtowrite = 0;
            char* out = scbin.AllocateGet(scbin.Size(), numtowrite);
            int numwritten = sock->Write(out, numtowrite, false);
            if (numwritten < 0) { break; }
            if (numwritten > 0) printf("Wrote %d to sock\n", numwritten);
            scbin.ReleaseGet(numwritten);
        }
        if (polldata[1].Out()) {
            // do blocking io
            int numtowrite = 0;
            char* out = scbout.AllocateGet(scbout.Size(), numtowrite);
            int numwritten = write(tout, out, numtowrite);
            if (numwritten < 0) {
                perror("Error on write to stdout");
                break;
            }
            scbout.ReleaseGet(numwritten);
        }
        if (polldata[2].In()) {
            int numtoread = 0;
            char* in = scbin.AllocatePut(BUFF_SIZE, numtoread);
            int numread = read(tin, in, numtoread);
            if (numread < 0) {
                perror("Error on read from stdin");
                break;
            }
            if (numread > 0) printf("Read %d from stdin\n", numread);
            scbin.ReleasePut(numread);
        }
    }
    StreamSocket::DeleteStreamSocket(sock);
	return 0;
}




