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

#include "FileHandle.h"
#include "ErrnoException.h"
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <algorithm>


int FileHandle::Poll(IteratorRef<FileHandle*> begin, IteratorRef<FileHandle*> end, double timeout) {
    fd_set rfd;
    fd_set wfd;
    FD_ZERO(&rfd);
    FD_ZERO(&wfd);
    int maxfd = 0;
    IteratorRef<FileHandle*> itr = begin;
    while (itr != end) {
        FileHandle *han = *itr;
        int fd = han->FD();
        if (fd < 0) {
            return -1;
        } else {
            bool set = false;
            if (!han->Readable()) {
                FD_SET(fd, &rfd);
                set = true;
            }
            if (!han->Writeable()) {
                FD_SET(fd, &wfd);
                set = true;
            }
            if (set) { maxfd = std::max(maxfd, fd); }
        }
        ++itr;
    }
    timeval tv;
    timeval *ptv = 0;
    if (timeout >= 0) {
        tv.tv_sec = (int)timeout;
        tv.tv_usec = (int)((timeout - tv.tv_sec) * 1e6);
        ptv = &tv;
    }
    int ret = select(maxfd + 1, &rfd, &wfd, 0, ptv);
    if (ret < 0) {
        if (errno == EINTR) {
            return 0;
        }
        throw ErrnoException();
    }
    itr = begin;
    while (itr != end) {
        FileHandle *han = *itr;
        int fd = han->FD();
        if (fd >= 0) {
            if (FD_ISSET(fd, &rfd)) {
                han->OnReadable();
            }
            if (FD_ISSET(fd, &wfd)) {
                han->OnWriteable();
            }
        }
        ++itr;
    }
    return ret;
}

FileHandle::FileHandle()
    : fd(-1), readable(false), writeable(true), eof(false)
{
}

FileHandle::FileHandle(int filed)
    : fd(filed), readable(false), writeable(true), eof(false)
{
}

FileHandle::~FileHandle() {
    if (fd != -1) { close(fd); }
    Reset();
}

int FileHandle::Poll(double timeout) {
    FileHandle *fh = this;
    return Poll(&fh, &fh + 1, timeout);
}

void FileHandle::SetBlocking(bool blocking) {
    SetBlocking(FD(), blocking);
}

void FileHandle::SetBlocking(int fd, bool blocking) {
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

bool FileHandle::IsBlocking() const {
    return IsBlocking(FD());
}

bool FileHandle::IsBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags) { throw ErrnoException(); }
    return !(flags & O_NONBLOCK);
}

void FileHandle::Reset() {
    AutoLock al(file_lock);
    readable = false;
    writeable = false;
    eof = false;
    fd = -1;
}

void FileHandle::Close() {
    AutoLock al(file_lock);
    if (fd != -1) {
        if (close(fd) != 0) {
            throw ErrnoException();
        }
        fd = -1;
    }
    readable = false;
    writeable = false;
    eof = false;
}

unsigned FileHandle::Read(void *ptr, unsigned len) {
    int filed;
    {
        AutoLock al(file_lock);
        if (eof || fd == -1) { return 0; }
        filed = fd;
    }
    unsigned bytesread = 0;
    int num = read(filed, ptr, len);
    if (num < 0) {
        int error = errno;
        switch (error) {
        case EAGAIN:
            Readable(false);
        case EINTR:
        case ENOMEM:
            break;
        default:
            throw ErrnoException(error);
        }
    } else if (num == 0 && len != 0) {
        AutoLock al(file_lock);
        eof = true;
        readable = false;
    } else {
        if (unsigned(num) < len) { Readable(false); }
        bytesread = num;
    }
    return bytesread;
}

unsigned FileHandle::Readv(const iovec *iov, int iovcnt) {
    int filed;
    {
        AutoLock al(file_lock);
        if (eof || fd == -1) { return 0; }
        filed = fd;
    }
    unsigned len = 0;
    for (int i = 0; i < iovcnt; ++i) {
        len += iov[i].iov_len;
    }
    unsigned bytesread = 0;
    int num = readv(filed, iov, iovcnt);
    if (num < 0) {
        int error = errno;
        switch (error) {
        case EAGAIN:
            Readable(false);
        case EINTR:
        case ENOMEM:
            break;
        default:
            throw ErrnoException(error);
        }
    } else if (num == 0 && len != 0) {
        AutoLock al(file_lock);
        eof = true;
        readable = false;
    } else {
        if (unsigned(num) < len) { Readable(false); }
        bytesread = num;
    }
    return bytesread;
}

unsigned FileHandle::Write(const void *ptr, unsigned len) {
    int filed;
    {
        AutoLock al(file_lock);
        if (fd == -1) { return 0; }
        filed = fd;
    }
    unsigned written = 0;
    int num = write(filed, ptr, len);
    if (num < 0) {
        int error = errno;
        switch (error) {
        case EAGAIN: // Would block
        //case EWOULDBLOCK:
            Writeable(false);
        case EINTR: // Returned because of signal
        case ENOMEM:
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
        if (unsigned(num) < len) { Writeable(false); }
        written = num;
    }
    return written;
}

unsigned FileHandle::Writev(const iovec *iov, int iovcnt) {
    int filed;
    {
        AutoLock al(file_lock);
        if (fd == -1) { return 0; }
        filed = fd;
    }
    unsigned written = 0;
    int num = writev(filed, iov, iovcnt);
    if (num < 0) {
        int error = errno;
        switch (error) {
        case EAGAIN:
            Writeable(false);
        case EINTR:
        case ENOMEM:
        case ENOBUFS:
            break;
        default:
            throw ErrnoException(error);
        }
    } else {
        unsigned len = 0;
        for (int i = 0; i < iovcnt; ++i) {
            len += iov[i].iov_len;
        }
        if (unsigned(num) < len) { Writeable(false); }
        written = num;
    }
    return written;
}

void FileHandle::Flush() {
    if (fsync(FD()) != 0) {
        throw ErrnoException();
    }
}

