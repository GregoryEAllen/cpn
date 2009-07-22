/** \file
 */

#include "Socket.h"

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
    bool loop = true;
    while (loop) {
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
            default:
                perror(__PRETTY_FUNCTION__);
                ret = false;
                loop = false;
                state = BAD;
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
            state = BAD;
            perror(__PRETTY_FUNCTION__);
            return false;
        }
    }
    state = LISTENING;
    return true;
}

StreamSocket* StreamSocket::Accept(bool block) {
    if (state == BAD) return 0;
    int flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags) { flags = 0; }
    if (!block) {
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    } else {
        fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    }
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
            case EWOULDBLOCK:
            case ECONNABORTED:
            case EINTR:
            case ENETDOWN:
            case EPROTO:
            case ENOPROTOPT:
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
                state = BAD;
                nfd = 0;
                perror(__PRETTY_FUNCTION__);
                break;
            }
        } else {
            sock = new StreamSocket(family, nfd, raddress);
        }
    }
    return sock;
}

int StreamScoket::Write(const void* ptr, const unsigned long size, bool block) {
    if (state == BAD) return -1;
    int flags = MSG_NOSIGNAL;
    if (!block) {
        flags =| MSG_DONTWAIT;
    }
    bool loop = true;
    int num = 0;
    while (loop) {
        num = send(df, ptr, size, flags);
        if (num < 0) {
            error = errno;
            switch (error) {
            case EINTR:
            case ENOMEM:
            case EAGAIN:
            case EWOULDBLOCK:
            case ENOBUFS: // Buffers are full
                num = 0;
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
                state = BAD;
                loop = false;
                perror(__PRETTY_FUNCTION__);
                break;
            }
        } else { loop = false; }
    }
    return num;
}

int StreamSocket::Read(void* ptr, const unsigned long size, bool block) {
    if (state == BAD) return -1;
    int flags = block ? 0 : MSG_DONTWAIT;
    int num = 0;
    bool loop = true;
    while (loop) {
        num = recv(fd, ptr, size, flags);
        if (num < 0) {
            error = errno;
            switch(error) {
            case EAGAIN:
            case EINTR:
            case ENOMEM:
                num = 0;
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
                state = BAD;
                loop = false;
                perror(__PRETTY_FUNCTION__);
                break;
            }
        } else { loop = false; }
    }
    return num;
}

int Poll(std::vector<PollData>& socks, int timeout) {
    return poll((pollfd*)&socks[0], socks.size(), timeout);
}

