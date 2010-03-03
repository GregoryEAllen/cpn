/** \file
 */

#include "ServerSocketHandle.h"
#include "StreamForwarder.h"
#include "ErrnoException.h"

#include <list>
#include <tr1/memory>

using std::tr1::shared_ptr;

class Controller {
public:
    typedef std::list<shared_ptr<StreamForwarder> > StreamList;
    Controller()
    {}

    void Read() {
        printf("%s\n",__PRETTY_FUNCTION__);
        if (!serv.Readable()) { return; }
        SocketAddress conaddr;
        int newfd = serv.Accept(conaddr);
        if (newfd >= 0) {
            printf("Accepted a connection from %s %s\n",
                    conaddr.GetHostName().c_str(),
                    conaddr.GetServName().c_str());

            sockets.push_back(shared_ptr<StreamForwarder>(new StreamForwarder()));
            sockets.back()->GetHandle().FD(newfd);
            sockets.back()->SetForward(sockets.back().get());
        }
        serv.Readable(false);
    }

    void Run() {
        printf("Started\n");
        serv.Readable(false);
        running = true;
        while (running) {
            std::vector<FileHandle*> polllist;
            StreamList::iterator itr = sockets.begin();
            while (itr != sockets.end()) {
                if (itr->get()->Good()) {
                    polllist.push_back(&itr->get()->GetHandle());
                    ++itr;
                } else {
                    printf("A connection closed\n");
                    itr = sockets.erase(itr);
                }
            }
            polllist.push_back(&serv);
            FileHandle::Poll(polllist.begin(), polllist.end(), -1);
            Read();
            itr = sockets.begin();
            while (itr != sockets.end()) {
                itr->get()->Read();
                itr->get()->Write();
                ++itr;
            }
            printf("tick\n");
        }
    }

    void Listen(const SockAddrList &addresses) {
        serv.Listen(addresses);
        serv.SetReuseAddr();
    }
private:
    StreamList sockets;
    ServerSocketHandle serv;
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




