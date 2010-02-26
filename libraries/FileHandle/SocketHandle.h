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
#ifndef SOCKETHANDLE_H
#define SOCKETHANDLE_H
#pragma once
#include "FileHandle.h"
#include "SocketAddress.h"

/**
 * \brief A FileHandler customized with some socket specific functionality
 * and functions.
 */
class SocketHandle : public FileHandler {
public:
    typedef FileHandler::AutoLock AutoLock;

    SocketHandle() {}
    SocketHandle(int nfd) : FileHandler(nfd) {}

    /** \brief Create a socket pair.
     * \param sock1 SocketHandle to fill with one of the created file descriptors
     * \param sock2 SocketHandle to fill with one of the created file descriptors
     */
    static void CreatePair(SocketHandle &sock1, SocketHandle &sock2);
    /** \brief Convenience function that returns actual file descriptors
     */
    static void CreatePair(int fd[2]);

    /**
     * \brief Create a new socket and try to connect to the given address
     * \param addr the address to connect to
     */
    void Connect(const SocketAddress &addr);

    /**
     * \brief Create a new socket and try to connect to one of the addresses
     * in the address list
     * \param addrs the addresses to try to connect to
     */
    void Connect(const SockAddrList &addrs);

    /**
     * \brief Shutdown the read end of this socket.
     * This does NOT close the socket! Any future attempt to read
     * from this socket will fail.
     */
    void ShutdownRead();
    /**
     * \brief Shutdown the write end of this socket.
     * Any future attempt to write to this socket will fail.
     * This is how you send an end of file down the socket.
     */
    void ShutdownWrite();

    /**
     * \param ptr pointer to memory to place read data
     * \param len maximum number of bytes to read
     * \param block If block true then wait for buffer to be full
     * if block false don't wait for anything
     * \return number of bytes read
     */
    unsigned Recv(void *ptr, unsigned len, bool block);

    /** Convenience structure for Send
     * so you can do things like
     * Send(ptr, len, SendOpts().Block(false).NoSignal(true));
     * Send(ptr, len, SendOpts()) is exactly the same as
     * Write(ptr, len)
     */
    struct SendOpts {
        SendOpts() : flags(0) {}
        SendOpts(int f) : flags(f) {}
#ifndef OS_DARWIN
        SendOpts &Block(bool block);
        SendOpts &NoSignal(bool sig);
        SendOpts &More(bool more);
#endif
        int flags;
    };

    /**
     * \param ptr pointer to bytes to write
     * \param len number of bytes to write
     * \param opts an option object (note that a blank option object this function is exactly the same as Write)
     * \return number of bytes written
     */
    unsigned Send(const void *ptr, unsigned len, const SendOpts &opts);

    /** \brief Get and clear any pending error
     * \return the pending error or 0 if none
     */
    int GetPendingError();

    void SetKeepAlive(int ka);
    int GetKeepAlive();
    /** \brief Set the linger socket options.
     * \param seconds number of seconds to linger, negative to turn off
     */
    void SetLingerTimeout(int seconds);
    /** \return number of seconds socket will linger, negative means off
     */
    int GetLinkgerTimeout();

    void SetReceiveBufferSize(int size);
    int GetReceiveBufferSize();
    void SetSendBufferSize(int size);
    int GetReceiveBufferSize();

    void SetReceiveTimeout(double timeout);
    void SetSendTimeout(double timeout);
    double GetReceiveTimeout();
    double GetSendTimeout();
private:
    bool Connect(const SocketAddress &addr, int &error);
};
#endif
