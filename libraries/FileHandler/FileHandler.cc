
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
    int ret = poll(&pollfdset[0], pollfdset.size(), (int)(timeout*1e6 + 0.5));
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
    } else if (num == 0) {
        eof = true;
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

void FileHandler::Flush() {
    if (fsync(fd) != 0) {
        throw ErrnoException();
    }
}
