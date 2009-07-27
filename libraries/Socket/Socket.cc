/** \file
 */

#include "Socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstdio>

using namespace Socket;

StreamSocket::StreamSocket(SockFamily fam)
: error(0), fd(-1), state(INIT), bound(false), family(fam) {
    fd = socket((int) family, SOCK_STREAM, 0);
    if (fd < 0) {
        perror(__PRETTY_FUNCTION__);
        error = errno;
        state = BAD;
    }
}

StreamSocket::StreamSocket(SockFamily fam, int fd_, SocketAddress raddress)
: error(0), fd(fd_), state(CONNECTED), family(fam), remoteaddress(raddress) {
    LookupLocalAddress();
}

StreamSocket::~StreamSocket() {
    Close();
}

bool StreamSocket::Connect(const SocketAddress &addr) {
    if (state == BAD) return false;
    bool loop = true;
    bool ret = true;
    state = CONNECTING;
    while (loop) {
        if (connect(fd, addr, addr) < 0) {
            error = errno;
            switch (error) {
            case EINTR:
                break;
            case EINPROGRESS: // non blocking and connection not completed yet
                // may want to change things to let non blocking connect.
            case EALREADY: // previous attempt has not completed

            case EAGAIN: // not enough resources
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
                perror(__PRETTY_FUNCTION__);
                ret = false;
                loop = false;
                Close();
                break;
            }
        } else {
           loop = false;
           remoteaddress = addr;
           state = CONNECTED;
           LookupLocalAddress();
           bound = true;
        }
    }
    return ret;
}

bool StreamSocket::Bind(const SocketAddress &addr) {
    if (state == BAD) return false;
    if (bound) return false;
    if (bind(fd, addr, addr) < 0) {
        error = errno;
        return false;
    }
    localaddress = addr;
    bound = true;
    return true;
}

bool StreamSocket::Listen(const int qsize) {
    if (state == BAD) return false;
    if (listen(fd, qsize) < 0) {
        error = errno;
        switch (error) {
        case EADDRINUSE:
        case EBADF:
        case ENOTSOCK:
        case EOPNOTSUPP:
        default:
            perror(__PRETTY_FUNCTION__);
            Close();
            return false;
        }
    }
    state = LISTENING;
    return true;
}

StreamSocket* StreamSocket::Accept(bool block) {
    if (state == BAD) return 0;
    SetBlocking(block);
    bool loop = true;
    int nfd = -1;
    StreamSocket *sock = 0;
    SocketAddress raddress;
    while (loop) {
        nfd = accept(fd, raddress, raddress);
        if (nfd < 0) {
            error = errno;
            switch (error) {
            case EAGAIN:
            //case EWOULDBLOCK: // Duplicate
            case ECONNABORTED:
            case EINTR:
            case ENETDOWN:
            case EPROTO:
            case ENOPROTOOPT:
            case EHOSTDOWN:
            case ENONET:
            case EHOSTUNREACH:
            case ENETUNREACH:
            case ENOBUFS:
            case ENOMEM:
            case EPERM:
            case EPIPE:
            case ETIMEDOUT:
                nfd = -1;
                if (!block) { loop = false; }
                break;
            case EOPNOTSUPP:
            case EBADF:
            case EINVAL:
            case ENFILE:
            case EMFILE:
            case ENOTSOCK:
            case EFAULT:
            default:
                loop = false;
                nfd = 0;
                perror(__PRETTY_FUNCTION__);
                Close();
                break;
            }
        } else {
            sock = new StreamSocket(family, nfd, raddress);
        }
    }
    return sock;
}

void StreamSocket::Close(void) {
    bool loop = true;
    while (loop && fd >= 0) {
        if (close(fd) < 0) {
            error = errno;
            switch(errno) {
            case EINTR: // retry
                break;
            case EBADF: // Bad file descriptor
                loop = false;
                break;
            case EIO: // IO error
            default:
                perror(__PRETTY_FUNCTION__);
                loop = false;
                break;
            }
        } else {
            loop = false;
        }
    }
    fd = -1;
    state = BAD;
}

int StreamSocket::Write(const void* ptr, const int size, bool block) {
    if (state == BAD) return -1;
    SetBlocking(block);
    int flags = MSG_NOSIGNAL;
    if (!block) {
        flags |= MSG_DONTWAIT;
    }
    bool loop = true;
    int written = 0;
    while (loop && written < size) {
        int num = send(fd, ((const char*)ptr + written), size - written, flags);
        if (num < 0) {
            error = errno;
            switch (error) {
            case EINTR: // Returned because of signal
            case ENOMEM:
            case EAGAIN: // Would block
            //case EWOULDBLOCK:
            case ENOBUFS: // Buffers are full
                if (!block) { loop = false; }
                break;
            case EACCES: // Permission denied
            case EBADF: // Bad file descriptor
            case ECONNRESET: // Connection reset by peer
            case EDESTADDRREQ:
            case EFAULT:
            case EINVAL:
            case EISCONN:
            case EMSGSIZE:
            case ENOTCONN:
            case ENOTSOCK:
            case EOPNOTSUPP:
            case EPIPE:
            case ETIMEDOUT:
            default:
                perror(__PRETTY_FUNCTION__);
                loop = false;
                written = -1;
                Close();
                break;
            }
        } else {
            written += num;
        }
    }
    return written;
}

int StreamSocket::Read(void* ptr, const int size, bool block) {
    if (state == BAD) return -1;
    SetBlocking(block);
    int flags = block ? 0 : MSG_DONTWAIT;
    int read = 0;
    bool loop = true;
    while (loop && read < size) {
        int num = recv(fd, ((char*)ptr + read), size - read, flags);
        if (num < 0) {
            error = errno;
            switch(error) {
            case EAGAIN:
            //case EWOULDBLOCK:
            case EINTR:
            case ENOMEM:
                if (!block) { loop = false; }
                break;
            case EBADF:
            case ECONNREFUSED:
            case EFAULT:
            case ENOTCONN:
            case ENOTSOCK:
            case ETIMEDOUT:
            case EPIPE:
            default:
                perror(__PRETTY_FUNCTION__);
                loop = false;
                read = -1;
                Close();
                break;
            }
        } else if (num == 0) {
            Close();
            read = -1;
            loop = false;
        } else {
            read += num;
        }
    }
    return read;
}

void StreamSocket::LookupLocalAddress(void) {
    *((socklen_t*)localaddress) = sizeof(sockaddr_storage);
    getsockname(fd, localaddress, localaddress);
}

void StreamSocket::LookupRemoteAddress(void) {
    *((socklen_t*)remoteaddress) = sizeof(sockaddr_storage);
    getpeername(fd, remoteaddress, remoteaddress);
}

void StreamSocket::SetBlocking(bool block) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags) { flags = 0; }
    if (!block) {
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    } else {
        fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    }
}

StreamSocket* StreamSocket::NewStreamSocket(SockFamily fam) {
    return new StreamSocket(fam);
}

void StreamSocket::DeleteStreamSocket(StreamSocket* sock) {
    delete sock;
}

int Socket::Poll(std::vector<PollData>& socks, int timeout) {
    return poll((pollfd*)&socks[0], socks.size(), timeout);
}

