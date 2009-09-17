/** \file
 */

#include "AsyncSocket.h"
#include "AsyncStream.h"
#include "StreamForwarder.h"
#include <string>
#include <cstdio>

using Async::Stream;
using Async::StreamSocket;
using Async::SocketAddress;
using Async::Descriptor;
using Async::SockAddrList;
using Async::SockPtr;
using Async::DescriptorPtr;

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Error must be: %s host port\n", argv[0]);
        return 0;
    }
    SockAddrList addresses;
    try {
        addresses = SocketAddress::CreateIP(argv[1], argv[2]);
    } catch (Async::StreamException &e) {
        printf("Lookup failed: %s\n", e.what());
        return 1;
    }
    SockPtr sock;
    for (SockAddrList::iterator itr = addresses.begin();
            itr != addresses.end(); ++itr) {
        printf("Attempting to connect to %s %s\n", itr->GetHostName().c_str(),
                itr->GetServName().c_str());
        try {
            sock = StreamSocket::Create(*itr);
        } catch (Async::StreamException &e) {
            printf("Connection failed: %s\n", e.what());
        }
        if (sock) break;
    }
    if (!sock) {
        return 1;
    }
    printf("Connected\n");
    
    DescriptorPtr in = Descriptor::Create(fileno(stdin));
    DescriptorPtr out = Descriptor::Create(fileno(stdout));
    StreamForwarder readside = StreamForwarder(Stream(in), Stream(sock));
    StreamForwarder writeside = StreamForwarder(Stream(sock), Stream(out));
    std::vector<DescriptorPtr> fds;
    fds.push_back(in);
    fds.push_back(out);
    fds.push_back(sock);
    try {
        while (readside.Good() && writeside.Good()) {
            Descriptor::Poll(fds, -1);
        }
    } catch (std::exception &e) {
        printf("Error: %s\n", e.what());
    }
	return 0;
}




