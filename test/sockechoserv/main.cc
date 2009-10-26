/** \file
 */

#include "ListenSockHandler.h"
#include "StreamForwarder.h"
#include "ErrnoException.h"

#include <list>
#include <tr1/memory>

using std::tr1::shared_ptr;

class Controller : public ListenSockHandler {
public:
    typedef std::list<shared_ptr<StreamForwarder> > StreamList;
    Controller()
    {
        Readable(true);
    }

    void OnRead() {
        printf("%s\n",__PRETTY_FUNCTION__);
        SocketAddress conaddr;
        int newfd = Accept(conaddr);
        if (newfd >= 0) {
            printf("Accepted a connection from %s %s\n",
                    conaddr.GetHostName().c_str(),
                    conaddr.GetServName().c_str());

            sockets.push_back(shared_ptr<StreamForwarder>(new StreamForwarder()));
            sockets.back()->FD(newfd);
            sockets.back()->SetForward(sockets.back().get());
        }
    }

    void OnError() {
        printf("%s\n",__PRETTY_FUNCTION__);
        running = false;
    }

    void OnInval() {
        printf("%s\n",__PRETTY_FUNCTION__);
        running = false;
    }
    
    void Run() {
        running = true;
        while (running) {
            std::vector<FileHandler*> polllist;
            StreamList::iterator itr = sockets.begin();
            while (itr != sockets.end()) {
                if (itr->get()->Good()) {
                    polllist.push_back(itr->get());
                    ++itr;
                } else {
                    printf("A connection closed\n");
                    itr = sockets.erase(itr);
                }
            }
            polllist.push_back(this);
            FileHandler::Poll(&polllist[0], polllist.size(), -1);
        }
    }
private:
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
    } catch (const ErrnoException &e) {
        printf("Invalid servname: %s\n", e.what());
        return 1;
    }

    Controller controller;
    controller.Listen(addresses);
    controller.Run();
	return 0;
}




