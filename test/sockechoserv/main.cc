/** \file
 */

#include "StreamForwarder.h"
#include "AsyncStream.h"
#include "AsyncSocket.h"

#include <utility>
#include <cstdio>
#include <tr1/memory>

using Async::SocketAddress;
using Async::SockAddrList;
using Async::StreamSocket;
using Async::Descriptor;
using Async::DescriptorPtr;
using Async::ListenSocket;
using Async::ListenSockPtr;
using Async::SockPtr;
using Async::Stream;
using std::tr1::shared_ptr;

class Controller : public sigc::trackable {
public:
    typedef std::vector<std::pair<shared_ptr<StreamForwarder>, DescriptorPtr > > StreamList;
    Controller(DescriptorPtr input, ListenSockPtr listensock)
    : in(input), lsock(listensock) {
        in->ConnectReadable(sigc::mem_fun(this, &Controller::ReturnTrue));
        in->ConnectOnRead(sigc::mem_fun(this, &Controller::StdRead));
        in->ConnectOnError(sigc::mem_fun(this, &Controller::Error));
        lsock->ConnectReadable(sigc::mem_fun(this, &Controller::ReturnTrue));
        lsock->ConnectOnRead(sigc::mem_fun(this, &Controller::ListenRead));
        lsock->ConnectOnError(sigc::mem_fun(this, &Controller::Error));
    }
    bool ReturnTrue() { return true; }
    void StdRead() {
        //printf("%s\n",__PRETTY_FUNCTION__);
        if (getchar() == 'q') {
            running = false;
        }
    }

    void ListenRead() {
        printf("%s\n",__PRETTY_FUNCTION__);
        SockPtr s = lsock->Accept();
        if (s) {
            printf("Accepted a connection from %s %s\n",
                    s->GetRemoteAddress().GetHostName().c_str(),
                    s->GetRemoteAddress().GetServName().c_str());

            sockets.push_back(std::make_pair(
                    shared_ptr<StreamForwarder>(new StreamForwarder(Stream(s), Stream(s))), s));
        }
    }

    void Error(int err) {
        printf("Error %d\n", err);
        running = false;
    }
    
    void Run() {
        running = true;
        while (running) {
            StreamList keepsocks;
            std::vector<DescriptorPtr> polllist;
            for (StreamList::iterator itr = sockets.begin();
                    itr != sockets.end(); ++itr) {
                if (*(itr->second)) {
                    keepsocks.push_back(*itr);
                    polllist.push_back(itr->second);
                } else {
                    printf("A connection closed\n");
                }
            }
            polllist.push_back(in);
            polllist.push_back(lsock);
            sockets = keepsocks;
            Descriptor::Poll(polllist, -1);
        }
    }
private:
    DescriptorPtr in;
    ListenSockPtr lsock;
    StreamList sockets;
    bool running;
};

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Error must be: %s port\n", argv[0]);
        return 0;
    }
    SockAddrList addresses;
    try {
        addresses = SocketAddress::CreateIPFromServ(argv[1]);
    } catch (Async::StreamException &e) {
        printf("Invalid servname: %s\n", e.what());
        return 1;
    }

    ListenSockPtr listensock;
    for (SockAddrList::iterator itr = addresses.begin();
            itr != addresses.end(); ++itr) {
        printf("Attempting bind to %s %s\n", itr->GetHostName().c_str(),
                itr->GetServName().c_str());
        try {
            listensock = ListenSocket::Create(*itr);
        } catch (Async::StreamException &e) {
            printf("Socket setup failed: %s\n", e.what());
        }
        if (listensock) break;
    }
    if (!listensock) {
        printf("Unable to create a listening socket.\n");
        return 1;
    }
    DescriptorPtr in = Descriptor::Create(fileno(stdin));
    Controller controller = Controller(in, listensock);
    controller.Run();
	return 0;
}




