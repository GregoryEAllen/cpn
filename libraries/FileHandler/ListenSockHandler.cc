
#include "ListenSockHandler.h"
#include "ErrnoException.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

void ListenSockHandler::Listen(const SocketAddress &addr) {
    int error = 0;
    if (!Listen(addr, error)) { throw ErrnoException(error); }
}

void ListenSockHandler::Listen(const SockAddrList &addrs) {
    int error = 0;
    bool success = false;
    for (SockAddrList::const_iterator itr = addrs.begin();
            itr != addrs.end(); ++itr) {
        success = Listen(*itr, error);
        if (success) break;
    }
    if (!success) throw ErrnoException(error);
}

int ListenSockHandler::Accept(SocketAddress &addr) {
    bool loop = true;
    int nfd = -1;
    while (loop) {
        addr.GetLen() = sizeof(sockaddr_storage);
        nfd = accept(fd, addr.GetAddr(), &addr.GetLen());
        if (nfd < 0) {
            int error = errno;
            switch (error) {
            case EAGAIN:
            //case EWOULDBLOCK: // Duplicate
            case ECONNABORTED:
            case EINTR:
            case ENETDOWN:
            case EPROTO:
            case ENOPROTOOPT:
            case EHOSTDOWN:
            //case ENONET:
            case EHOSTUNREACH:
            case ENETUNREACH:
            case ENOBUFS:
            case ENOMEM:
            case EPERM:
            case EPIPE:
            case ETIMEDOUT:
                nfd = -1;
                loop = false;
                break;
            case EOPNOTSUPP:
            case EBADF:
            case EINVAL:
            case ENFILE:
            case EMFILE:
            case ENOTSOCK:
            case EFAULT:
            default:
                throw ErrnoException(error);
            }
        } else {
            loop = false;
        }
    }
    return nfd;
}

bool ListenSockHandler::Listen(const SocketAddress &addr, int &error) {
    SocketAddress address = addr;
    fd = socket(address.Family(), SOCK_STREAM, 0);
    if (fd < 0) {
        error = errno;
        return false;
    }
    if (bind(fd, address.GetAddr(), address.GetLen()) < 0) {
        error = errno;
        return false;
    }
    if (listen(fd, 10) < 0) {
        error = errno;
        return false;
    }
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    return true;
}


