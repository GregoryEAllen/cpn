/** \file
 */

#include "SocketAddress.h"
#include "StreamForwarder.h"
#include "ErrnoException.h"
#include <string>
#include <cstdio>


int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Error must be: %s host port\n", argv[0]);
        return 0;
    }
    SockAddrList addresses;
    try {
        addresses = SocketAddress::CreateIP(argv[1], argv[2]);
    } catch (const ErrnoException &e) {
        printf("Lookup failed: %s\n", e.what());
        return 1;
    }

    
    StreamForwarder sock;
    sock.GetHandle().Connect(addresses);
    printf("Connected\n");

    StreamForwarder outside;
    outside.GetHandle().FD(fileno(stdout));
    sock.SetForward(&outside);

    StreamForwarder inside;
    inside.GetHandle().FD(fileno(stdin));
    inside.SetForward(&sock);

    std::vector<FileHandle*> fds;
    fds.push_back(&sock.GetHandle());
    fds.push_back(&outside.GetHandle());
    fds.push_back(&inside.GetHandle());
    try {
        while (sock.Good() && inside.Good()) {
            FileHandle::Poll(fds.begin(), fds.end(), -1);
            if (inside.GetHandle().Readable()) {
                printf("stdin readable\n");
            }
            inside.Read();
            if (sock.GetHandle().Writeable()) {
                printf("sock writeable\n");
            }
            sock.Write();
            if (sock.GetHandle().Readable()) {
                printf("sock readable\n");
            }
            sock.Read();
            if (outside.GetHandle().Writeable()) {
                printf("stdout writeable\n");
            }
            outside.Write();
        }
        sock.GetHandle().Close();
    } catch (std::exception &e) {
        printf("Error: %s\n", e.what());
    }
	return 0;
}




