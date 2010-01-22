//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \author John Bridgman
 */

#include "ListenSockHandler.h"
#include "ErrnoException.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

void ListenSockHandler::Listen(const SocketAddress &addr, int queuelength) {
    int error = 0;
    if (!Listen(addr, queuelength, error)) { throw ErrnoException(error); }
}

void ListenSockHandler::Listen(const SockAddrList &addrs, int queuelength) {
    int error = 0;
    bool success = false;
    for (SockAddrList::const_iterator itr = addrs.begin();
            itr != addrs.end(); ++itr) {
        success = Listen(*itr, queuelength, error);
        if (success) break;
    }
    if (!success) throw ErrnoException(error);
}

int ListenSockHandler::Accept(SocketAddress &addr) {
    return Accept(&addr);
}

int ListenSockHandler::Accept() {
    return Accept(0);
}
int ListenSockHandler::Accept(SocketAddress *addr) {
    bool loop = true;
    int nfd = -1;
    while (loop) {
        if (addr) {
            addr->GetLen() = sizeof(sockaddr_storage);
            nfd = accept(fd, addr->GetAddr(), &addr->GetLen());
        } else {
            nfd = accept(fd, 0, 0);
        }
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

bool ListenSockHandler::Listen(const SocketAddress &addr, int queuelength, int &error) {
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
    if (listen(fd, queuelength) < 0) {
        error = errno;
        return false;
    }
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    return true;
}


