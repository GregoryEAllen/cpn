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

#include "AsyncStream.h"
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

static const char UNKNOWN_ERROR[] = "Unknown error";

struct PollFd : public pollfd {
public:
    PollFd(int fid, bool in, bool out) {
        events = 0;
        fd = fid;
        Reset(in, out);
    }
    void Reset(bool in, bool out) {
        events = in ? (events | POLLIN) : (events & ~POLLIN);
        events = out ? (events | POLLOUT) : (events & ~POLLOUT);
        revents = 0;
    }
    bool In(void) { return (0 != (revents&POLLIN)); }
    bool Out(void) { return (0 != (revents&POLLOUT)); }
    bool Err(void) { return (0 != (revents&POLLERR)); }
    bool Hup(void) { return (0 != (revents&POLLHUP)); }
    bool Inval(void) { return (0 != (revents&POLLNVAL)); }
};


namespace Async {

    StreamException::StreamException(int err) throw() : error(err), errstr(256,
            '\0') { do { char *str = strerror_r(error, &errstr[0],
                errstr.size());
            // Wierdness with different versions of strerror... From the man
            // page:
            //
            //  The strerror() and strerror_r() functions return the
            //  appropriate error description string, or an "Unknown error nnn"
            //  message if the error number is unknown.
            //
            //  The XSI-compliant strerror_r() function returns 0 on success;
            //  on error, -1 is returned and errno is set to indicate the
            //  error.
            //
            if (str == (char*)-1) {
                if (errno == ERANGE) {
                    errstr.resize(2*errstr.size(), '\0');
                } else {
                    errstr.assign(UNKNOWN_ERROR, UNKNOWN_ERROR+sizeof(UNKNOWN_ERROR));
                    break;
                }
            } else if (str == 0) {
                break;
            } else {
                errstr.assign(str, str + strlen(str));
                break;
            }
        } while (true);
    }

    StreamException::StreamException(const char *msg, int err) throw()
    : error(err) {
        while (*msg != '\0') {
            errstr.push_back(*msg);
            ++msg;
        }
        errstr.push_back('\0');
    }

    StreamException::~StreamException() throw() {
    }

    const char* StreamException::what() const throw() {
        return &errstr[0];
    }

    DescriptorPtr Descriptor::Create(int fd) {
        return DescriptorPtr(new Descriptor(fd));
    }

    Descriptor::Descriptor() : fd(-1), eof(false) {
        SetNonBlocking(true);
    }

    Descriptor::Descriptor(int fd_) : fd(fd_), eof(false) {
        SetNonBlocking(true);
    }

    Descriptor::~Descriptor() throw() {
        try {
            Close();
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void Descriptor::Close() throw(StreamException) {
        bool loop = true;
        while (loop && fd >= 0) {
            if (close(fd) < 0) {
                int error = errno;
                switch(error) {
                case EINTR: // retry
                    break;
                case EBADF: // Bad file descriptor
                    loop = false;
                    break;
                case EIO: // IO error
                default:
                    throw StreamException(error);
                    break;
                }
            } else {
                loop = false;
            }
        }
        fd = -1;
    }

    void Descriptor::SetNonBlocking(bool nonblocking) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (-1 == flags) { flags = 0; }
        if (nonblocking) {
            fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        } else {
            fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
        }
    }

    int Descriptor::Poll(std::vector<DescriptorPtr> &fildes, int timeout) {
        std::vector<PollFd> pollfildes;
        for (std::vector<DescriptorPtr>::iterator itr = fildes.begin();
                itr != fildes.end(); ++itr) {
            pollfildes.push_back(PollFd((*itr)->fd, (*itr)->readable(), (*itr)->writeable()));
        }
        int ret = poll(&pollfildes[0], pollfildes.size(), timeout);
        if (ret > 0) {
            std::vector<DescriptorPtr>::iterator ditr = fildes.begin();
            std::vector<PollFd>::iterator pollitr = pollfildes.begin();
            while (ditr != fildes.end()) {
                if (pollitr->In()) { (*ditr)->read(); }
                if (pollitr->Out()) { (*ditr)->write(); }
                if (pollitr->Err() || pollitr->Hup() || pollitr->Inval()) {
                    (*ditr)->error(pollitr->revents);
                }
                ++ditr;
                ++pollitr;
            }
        } else {
            int error = errno;
            switch (error) {
            case EINTR:
                break;
            case EINVAL:
            case ENOMEM:
            case EFAULT:
            case EBADF:
            default:
                throw StreamException(error);
            }
        }
        return ret;
    }

    Stream::Stream(DescriptorPtr desc) throw()
        : descriptor(desc) {}

    unsigned Stream::Read(void *ptr, unsigned len) throw(StreamException) {
        if (descriptor->Eof()) { return 0; }
        bool loop = true;
        unsigned bytesread = 0;
        while (loop && bytesread < len) {
            int num = read(descriptor->fd, ((char*)ptr+bytesread), len - bytesread);
            if (num < 0) {
                int error = errno;
                switch (error) {
                case EINTR:
                case EAGAIN:
                case ENOMEM:
                    loop = false;
                    break;
                default:
                    throw StreamException(error);
                }
            } else if (num == 0) {
                descriptor->SetEof();
                loop = false;
            } else {
                bytesread += num;
            }
        }
        return bytesread;
    }

    unsigned Stream::Write(const void *ptr, unsigned len) throw(StreamException) {
        bool loop = true;
        unsigned written = 0;
        while (loop && written < len) {
            int num = write(descriptor->fd, ((const char*)ptr + written), len - written);
            if (num < 0) {
                int error = errno;
                switch (error) {
                case EINTR: // Returned because of signal
                case ENOMEM:
                case EAGAIN: // Would block
                //case EWOULDBLOCK:
                case ENOBUFS: // Buffers are full
                    loop = false;
                    break;
                case EPIPE:
                case EBADF:
                case EFAULT:
                case EFBIG:
                case EINVAL:
                case EIO:
                case ENOSPC:
                default:
                    throw StreamException(error);
                }
            } else {
                written += num;
            }
        }
        return written;
    }

}

