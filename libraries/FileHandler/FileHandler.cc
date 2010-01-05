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

#include "FileHandler.h"
#include "ErrnoException.h"
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


int FileHandler::Poll(FileHandler **fileds, unsigned numfds, double timeout) {
    std::vector<pollfd> pollfdset;
    for (unsigned i = 0; i < numfds; ++i) {
        pollfd pfd = {0};
        pfd.fd = fileds[i]->FD();
        if (fileds[i]->Readable()) {
            pfd.events |= POLLIN;
        }
        if (fileds[i]->Writeable()) {
            pfd.events |= POLLOUT;
        }
        pollfdset.push_back(pfd);
    }

    int timeoutint = -1;
    if (timeout >= 0) {
        timeoutint = (int)(timeout*1e3 + 0.5);
    }
    int ret = poll(&pollfdset[0], pollfdset.size(), timeoutint);
    if (ret > 0) {
        for (unsigned i = 0; i < numfds; ++i) {
            if (pollfdset[i].revents & POLLNVAL) { fileds[i]->OnInval(); }
            if (pollfdset[i].revents & POLLHUP) { fileds[i]->OnHup(); }
            if (pollfdset[i].revents & POLLERR) { fileds[i]->OnError(); }
            if (pollfdset[i].revents & POLLOUT) { fileds[i]->OnWrite(); }
            if (pollfdset[i].revents & POLLIN) { fileds[i]->OnRead(); }
        }
    } else if (ret < 0) {
        throw ErrnoException();
    }
    return ret;
}

FileHandler::FileHandler()
    : fd(-1), readable(false), writeable(false), eof(false)
{
}

FileHandler::FileHandler(int filed)
    : fd(filed), readable(false), writeable(false), eof(false)
{
}

FileHandler::~FileHandler() {
    if (fd != -1) { close(fd); }
    Reset();
}

int FileHandler::Poll(double timeout) {
    FileHandler *t = this;
    return Poll(&t, 1, timeout);
}

void FileHandler::SetBlocking(bool blocking) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags) { throw ErrnoException(); }
    if (blocking) {
        flags &= ~O_NONBLOCK;
    } else {
        flags |= O_NONBLOCK;
    }
    if (fcntl(fd, F_SETFL, flags) != 0) {
        throw ErrnoException();
    }
}

bool FileHandler::IsBlocking() const {
    int flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags) { throw ErrnoException(); }
    return !(flags & O_NONBLOCK);
}

void FileHandler::Reset() {
    readable = false;
    writeable = false;
    eof = false;
    fd = -1;
}

void FileHandler::Close() {
    if (fd != -1) {
        if (close(fd) != 0) {
            throw ErrnoException();
        }
        fd = -1;
    }
}

unsigned FileHandler::Read(void *ptr, unsigned len) {
    if (eof) { return 0; }
    unsigned bytesread = 0;
    int num = read(fd, ptr, len);
    if (num < 0) {
        int error = errno;
        switch (error) {
        case EINTR:
        case EAGAIN:
        case ENOMEM:
            break;
        default:
            throw ErrnoException(error);
        }
    } else if (num == 0 && len != 0) {
        eof = true;
    } else {
        bytesread = num;
    }
    return bytesread;
}

unsigned FileHandler::Readv(const iovec *iov, int iovcnt) {
    if (eof) { return 0; }
    unsigned bytesread = 0;
    int num = readv(fd, iov, iovcnt);
    if (num < 0) {
        int error = errno;
        switch (error) {
        case EINTR:
        case EAGAIN:
        case ENOMEM:
            break;
        default:
            throw ErrnoException(error);
        }
    } else if (num == 0) {
        bool zero = true;
        for (int i = 0; i < iovcnt; ++i) {
            if (iov[i].iov_len > 0) {
                zero = false;
                break;
            }
        }
        if (zero) {
            eof = true;
        }
    } else {
        bytesread = num;
    }
    return bytesread;
}

unsigned FileHandler::Write(const void *ptr, unsigned len) {
    unsigned written = 0;
    int num = write(fd, ptr, len);
    if (num < 0) {
        int error = errno;
        switch (error) {
        case EINTR: // Returned because of signal
        case ENOMEM:
        case EAGAIN: // Would block
        //case EWOULDBLOCK:
        case ENOBUFS: // Buffers are full
            break;
        case EPIPE:
        case EBADF:
        case EFAULT:
        case EFBIG:
        case EINVAL:
        case EIO:
        case ENOSPC:
        default:
            throw ErrnoException(error);
        }
    } else {
        written = num;
    }
    return written;
}

unsigned FileHandler::Writev(const iovec *iov, int iovcnt) {
    unsigned written = 0;
    int num = writev(fd, iov, iovcnt);
    if (num < 0) {
        int error = errno;
        switch (error) {
        case EINTR:
        case ENOMEM:
        case EAGAIN:
        case ENOBUFS:
            break;
        default:
            throw ErrnoException(error);
        }
    } else {
        written = num;
    }
    return written;
}

void FileHandler::Flush() {
    if (fsync(fd) != 0) {
        throw ErrnoException();
    }
}
