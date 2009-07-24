/** \file
 * An abstraction of a connection protocol.
 */

#ifndef SOCKET_SOCKET_H
#define SOCKET_SOCKET_H
#pragma once

#include "SocketAddress.h"
#include "RefCounter.h"
#include <poll.h>

/*
 * Sockets have the following associated with them:
 * A domain or protocol family
 * A type.
 * And a specific protocol type (zero for almost all)
 *
 * An address has associted with it:
 * A type (this is the same as the type above!)
 * type dependent data
 * Most have some kind of standard address string
 * representation. Some have an additional integer
 * port number.
 *
 * A socket also has a set of modes and options.
 * A set of these modes are valid for all sockets
 * and each type has some of its own modes.
 */
/**
 * An abstraction of the socket API to hide
 * some details of what is going on. This abstraction
 * only supports stream based sockets on ipv4 and ipv6.
 *
 * This class is not thread safe. But there is no global
 * state so as long as a reference is not passed between
 * threads things are fine.
 *
 * This class keeps all it's data in a referenced counted
 * structure that it passes around. This is so that
 * functions like Accept can return by value a new socket.
 *
 * This is a simple abstraction. Therefor, most errors even
 * some recoverable errors are treated as
 * fatal. When an error happens one can find out what happened
 * by calling LastError. In almost all cases one must create a new
 * socket and start over.
 */
namespace Socket {
    class PollData;

    class StreamSocket {
    public:

        enum State_t {
            INIT,
            BAD,
            LISTENING,
            CONNECTING,
            CONNECTED
        };

        ~StreamSocket();

        bool Connect(const SocketAddress &addr);
        bool Bind(const SocketAddress &addr);
        bool Listen(const int qsize);
        StreamSocket* Accept(bool block);
        void Close(void);

        int Write(const void* ptr, const int size, bool block);

        int Read(void* ptr, const int size, bool block);

        SocketAddress GetLocalAddress(void) { return localaddress; }
        SocketAddress GetRemoteAddress(void) { return remoteaddress; }

        int LastError(void) const { return error; }
    protected:
        StreamSocket(SockFamily fam);
        StreamSocket(SockFamily fam, int fd, SocketAddress raddress);
        void LookupLocalAddress(void);
        void LookupRemoteAddress(void);
    private:
        StreamSocket(const StreamSocket& osock);
        StreamSocket &operator=(const StreamSocket& osock);
        void SetBlocking(bool block);

        int error;
        int fd;
        State_t state;
        bool bound;
        SockFamily family;
        SocketAddress localaddress;
        SocketAddress remoteaddress;

        friend StreamSocket* NewStreamSocket(SockFamily fam);
        friend class PollData;
    };

    StreamSocket* NewStreamSocket(SockFamily fam);
    void DeleteStreamScocket(StreamSocket* sock);
    class PollData : public pollfd {
    public:
        PollData(StreamSocket* sock, bool in, bool out) : revents(0) {
            fd = sock->fd;
            events = (in ? POLLIN : 0) | (out ? POLLOUT : 0);
        }
        bool In(void) { return (0 != (revents&POLLIN)); }
        bool Out(void) { return (0 != (revents&POLLOUT)); }
        bool Err(void) { return (0 != (revents&POLLERR)); }
        bool Hup(void) { return (0 != (revents&POLLHUP)); }
    };
    int Poll(std::vector<PollData>& socks, int timeout);
}

#endif

