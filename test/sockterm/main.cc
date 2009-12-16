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
    sock.Connect(addresses);
    printf("Connected\n");

    StreamForwarder outside;
    outside.FD(fileno(stdout));
    sock.SetForward(&outside);

    StreamForwarder inside;
    inside.FD(fileno(stdin));
    inside.SetForward(&sock);

    std::vector<FileHandler*> fds;
    fds.push_back(&sock);
    fds.push_back(&outside);
    fds.push_back(&inside);
    try {
        while (sock.Good() && inside.Good()) {
            FileHandler::Poll(&fds[0], fds.size(), -1);
        }
        sock.Close();
    } catch (std::exception &e) {
        printf("Error: %s\n", e.what());
    }
	return 0;
}




