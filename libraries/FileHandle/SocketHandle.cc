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

#include "SocketHandle.h"
#include "ErrnoException.h"
#include "Assert.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

void SocketHandle::CreatePair(SocketHandle &sock1, SocketHandle &sock2) {
    ASSERT(sock1.FD() == -1, "sock1 already connected");
    ASSERT(sock2.FD() == -1, "sock2 already connected");
    int pair[2];
    CreatePair(pair);
    sock1.FD(pair[0]);
    sock2.FD(pair[1]);
}

void SocketHandle::CreatePair(int fd[2]) {
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0) {
        throw ErrnoException(errno);
    }
}

void SocketHandle::Connect(const SocketAddress &addr) {
    int error = 0;
    if (!Connect(addr, error)) { throw ErrnoException(error); }
}

void SocketHandle::Connect(const SockAddrList &addrs) {
    int error = 0;
    bool success = false;
    for (SockAddrList::const_iterator itr = addrs.begin();
            itr != addrs.end(); ++itr) {
        success = Connect(*itr, error);
        if (success) break;
    }
    if (!success) throw ErrnoException(error);
}

bool SocketHandle::Connect(const SocketAddress &addr, int &error) {
    ASSERT(Closed(), "Already connected!");
    int nfd = socket(addr.Family(), SOCK_STREAM, 0);
    if (nfd < 0) {
        error = errno;
        return false;
    }
    SocketAddress address = addr;
    bool loop = true;
    while (loop) {
        if (connect(nfd, address.GetAddr(), address.GetLen()) < 0) {
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
                close(nfd);
                nfd = -1;
                return false;
            }
        } else {
           loop = false;
        }
    }
    FD(nfd);
    return true;
}

void SocketHandle::ShutdownRead() {
    if (shutdown(FD(), SHUT_RD) != 0) {
        throw ErrnoException();
    }
}

void SocketHandle::ShutdownWrite() {
    if (shutdown(FD(), SHUT_WR) != 0) {
        throw ErrnoException();
    }
}

unsigned SocketHandle::Recv(void *ptr, unsigned len, bool block) {
    int filed;
    {
        ALock al(file_lock);
        if (eof || fd == -1) { return 0; }
        filed = fd;
    }
    int flags = 0;
    if (block) {
        flags |= MSG_WAITALL;
    } else {
        flags |= MSG_DONTWAIT;
    }
    unsigned bytesread = 0;
    int num = recv(filed, ptr, len, flags);
    if (num > 0) {
        if (unsigned(num) < len) { Readable(false); }
        bytesread = num;
    } else if (num == 0 && len != 0) {
        ALock al(file_lock);
        eof = true;
        readable = false;
    } else if (num < 0) {
        int error = errno;
        switch (error) {
        case EAGAIN: // Ether non blocking or timed out
            Readable(false);
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
SocketHandle::SendOpts &SocketHandle::SendOpts::Block(bool block) {
    if (block) { flags &= ~MSG_DONTWAIT; }
    else { flags |= MSG_DONTWAIT; }
    return *this;
}

SocketHandle::SendOpts &SocketHandle::SendOpts::NoSignal(bool sig) {
    if (sig) { flags &= ~MSG_NOSIGNAL; }
    else { flags |= MSG_NOSIGNAL; }
    return *this;
}

SocketHandle::SendOpts &SocketHandle::SendOpts::More(bool more) {
    if (more) { flags |= MSG_MORE; }
    else { flags &= ~MSG_MORE; }
    return *this;
}
#endif

unsigned SocketHandle::Send(const void *ptr, unsigned len, const SendOpts &opts) {
    int filed;
    {
        ALock al(file_lock);
        if (fd == -1) { return 0; }
        filed = fd;
    }
    unsigned written = 0;
    int num = send(filed, ptr, len, opts.flags);
    if (num < 0) {
        int error = errno;
        switch (error) {
        case EAGAIN:
            Writeable(false);
        case EINTR:
        case ENOMEM:
            break;
        default:
            throw ErrnoException(error);
        }
    } else {
        if (unsigned(num) < len) { Writeable(false); }
        written = num;
    }
    return written;
}

int SocketHandle::GetPendingError() {
    int err = 0;
    socklen_t len = sizeof(err);
    if (getsockopt(FD(), SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
        throw ErrnoException();
    }
    return err;
}

void SocketHandle::SetKeepAlive(int ka) {
    if (setsockopt(FD(), SOL_SOCKET, SO_KEEPALIVE, &ka, sizeof(ka)) < 0) {
        throw ErrnoException();
    }
}

int SocketHandle::GetKeepAlive() {
    int ka;
    socklen_t len = sizeof(ka);
    if (getsockopt(FD(), SOL_SOCKET, SO_KEEPALIVE, &ka, &len) < 0) {
        throw ErrnoException();
    }
    return ka;
}

void SocketHandle::SetLingerTimeout(int seconds) {
    linger l = {0};
    if (seconds > 0) {
        l.l_onoff = 1;
        l.l_linger = seconds;
    }
    if (setsockopt(FD(), SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
        throw ErrnoException();
    }
}

int SocketHandle::GetLingerTimeout() {
    linger l = {0};
    socklen_t len = sizeof(l);
    if (getsockopt(FD(), SOL_SOCKET, SO_LINGER, &l, &len) < 0) {
        throw ErrnoException();
    }
    if (l.l_onoff) {
        return -1;
    } else {
        return l.l_linger;
    }
}

void SocketHandle::SetReceiveBufferSize(int size) {
    if (setsockopt(FD(), SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) < 0) {
        throw ErrnoException();
    }
}

int SocketHandle::GetReceiveBufferSize() {
    int size;
    socklen_t len = sizeof(size);
    if (getsockopt(FD(), SOL_SOCKET, SO_RCVBUF, &size, &len) < 0) {
        throw ErrnoException();
    }
    return size;
}

void SocketHandle::SetSendBufferSize(int size) {
    if (setsockopt(FD(), SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) < 0) {
        throw ErrnoException();
    }
}

int SocketHandle::GetSendBufferSize() {
    int size;
    socklen_t len = sizeof(size);
    if (getsockopt(FD(), SOL_SOCKET, SO_SNDBUF, &size, &len) < 0) {
        throw ErrnoException();
    }
    return size;
}


void SocketHandle::SetReceiveTimeout(double timeout) {
    timeval tv = {0};
    tv.tv_sec = (int)timeout;
    tv.tv_usec = (int)((timeout - tv.tv_sec) * 1e6);
    if (setsockopt(FD(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        throw ErrnoException();
    }
}

void SocketHandle::SetSendTimeout(double timeout) {
    timeval tv = {0};
    tv.tv_sec = (int)timeout;
    tv.tv_usec = (int)((timeout - tv.tv_sec) * 1e6);
    if (setsockopt(FD(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
        throw ErrnoException();
    }
}

double SocketHandle::GetReceiveTimeout() {
    timeval tv = {0};
    socklen_t len = sizeof(tv);
    if (getsockopt(FD(), SOL_SOCKET, SO_RCVTIMEO, &tv, &len) < 0) {
        throw ErrnoException();
    }
    return ((double)tv.tv_sec) + (((double)tv.tv_usec)*1e-6);
}

double SocketHandle::GetSendTimeout() {
    timeval tv = {0};
    socklen_t len = sizeof(tv);
    if (getsockopt(FD(), SOL_SOCKET, SO_SNDTIMEO, &tv, &len) < 0) {
        throw ErrnoException();
    }
    return ((double)tv.tv_sec) + (((double)tv.tv_usec)*1e-6);
}


