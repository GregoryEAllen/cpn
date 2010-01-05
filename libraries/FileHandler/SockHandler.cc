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

#include "SockHandler.h"
#include "ErrnoException.h"
#include "Assert.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

void SockHandler::CreatePair(SockHandler &sock1, SockHandler &sock2) {
    ASSERT(sock1.FD() == -1, "sock1 already connected");
    ASSERT(sock2.FD() == -1, "sock2 already connected");
    int pair[2];
    CreatePair(pair);
    sock1.FD(pair[0]);
    sock2.FD(pair[1]);
}

void SockHandler::CreatePair(int fd[2]) {
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0) {
        throw ErrnoException(errno);
    }
}

void SockHandler::Connect(const SocketAddress &addr) {
    int error = 0;
    if (!Connect(addr, error)) { throw ErrnoException(error); }
}

void SockHandler::Connect(const SockAddrList &addrs) {
    int error = 0;
    bool success = false;
    for (SockAddrList::const_iterator itr = addrs.begin();
            itr != addrs.end(); ++itr) {
        success = Connect(*itr, error);
        if (success) break;
    }
    if (!success) throw ErrnoException(error);
}

bool SockHandler::Connect(const SocketAddress &addr, int &error) {
    ASSERT(fd == -1, "Already connected!");
    fd = socket(addr.Family(), SOCK_STREAM, 0);
    if (fd < 0) {
        error = errno;
        return false;
    }
    SocketAddress address = addr;
    bool loop = true;
    while (loop) {
        if (connect(fd, address.GetAddr(), address.GetLen()) < 0) {
            error = errno;
            switch (error) {
            case EINTR:
            case EAGAIN: // not enough resources
                break;
            case EINPROGRESS: // non blocking and connection not completed yet
                // may want to change things to let non blocking connect.
            case EALREADY: // previous attempt has not completed
            case EACCES:
            case EPERM:
            case EADDRINUSE:
            case EAFNOSUPPORT:
            case EBADF:
            case ECONNREFUSED:
            case EFAULT:
            case EISCONN:
            case ENETUNREACH:
            case ENOTSOCK:
            case ETIMEDOUT:
            default:
                close(fd);
                fd = -1;
                return false;
            }
        } else {
           loop = false;
        }
    }
    return true;
}

void SockHandler::ShutdownRead() {
    if (shutdown(fd, SHUT_RD) != 0) {
        throw ErrnoException();
    }
}

void SockHandler::ShutdownWrite() {
    if (shutdown(fd, SHUT_WR) != 0) {
        throw ErrnoException();
    }
}

unsigned SockHandler::Recv(void *ptr, unsigned len, bool block) {
    if (eof) { return 0; }
    int flags = 0;
    if (block) {
        flags |= MSG_WAITALL;
    } else {
        flags |= MSG_DONTWAIT;
    }
    unsigned bytesread = 0;
    int num = recv(fd, ptr, len, flags);
    if (num > 0) {
        bytesread = num;
    } else if (num == 0 && len != 0) {
        eof = true;
    } else if (num < 0) {
        int error = errno;
        switch (error) {
        case EAGAIN: // Ether non blocking or timed out
        case EINTR: // Interrupted by an interrupt
        case ENOMEM:
            // report nothing received
            break;
        default:
            throw ErrnoException(error);
        }
    }
    return bytesread;
}

#ifndef OS_DARWIN
SockHandler::SendOpts &SockHandler::SendOpts::Block(bool block) {
    if (block) { flags &= ~MSG_DONTWAIT; }
    else { flags |= MSG_DONTWAIT; }
    return *this;
}

SockHandler::SendOpts &SockHandler::SendOpts::NoSignal(bool sig) {
    if (sig) { flags &= ~MSG_NOSIGNAL; }
    else { flags |= MSG_NOSIGNAL; }
    return *this;
}

SockHandler::SendOpts &SockHandler::SendOpts::More(bool more) {
    if (more) { flags |= MSG_MORE; }
    else { flags &= ~MSG_MORE; }
    return *this;
}
#endif

unsigned SockHandler::Send(const void *ptr, unsigned len, const SendOpts &opts) {
    unsigned written = 0;
    int num = send(fd, ptr, len, opts.flags);
    if (num < 0) {
        int error = errno;
        switch (error) {
        case EAGAIN:
        case EINTR:
        case ENOMEM:
            break;
        default:
            throw ErrnoException(error);
        }
    } else {
        written = num;
    }
    return written;
}


