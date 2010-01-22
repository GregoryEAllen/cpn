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
#ifndef LISTENSOCKETHANDLER_H
#define LISTENSOCKETHANDLER_H
#pragma once

#include "FileHandler.h"
#include "SocketAddress.h"

class ListenSockHandler : public FileHandler {
public:

    ListenSockHandler() {}
    ListenSockHandler(int nfd) : FileHandler(nfd) {}
    ListenSockHandler(const SocketAddress &addr, int queuelength = 256) { Listen(addr, queuelength); }
    ListenSockHandler(const SockAddrList &addrs, int queuelength = 256) { Listen(addrs, queuelength); }

    /**
     * Open a new socket and try to listen on the given
     * SocketAddress.
     */
    void Listen(const SocketAddress &addr, int queuelength = 256);

    /**
     * Open a new socket and try to listen on one of the
     * addresses in the given list.
     */
    void Listen(const SockAddrList &addrs, int queuelength = 256);

    /**
     * Accept an incoming connection.
     * \param addr a SocketAddress object to fill with the address of the connecting peer.
     * \return -1 if no connection >=0 on success
     * \throws ErrnoException for errors
     */
    int Accept(SocketAddress &addr);
    /**
     * Accept an incoming connection
     */
    int Accept();

    /**
     * When a connection is ready OnRead will be called by Poll
     */
    virtual void OnRead() = 0;
    virtual void OnError() = 0;
    virtual void OnInval() = 0;

private:
    // These can't happen to a listening socket
    void OnWrite() {}
    void OnHup() {}

    bool Listen(const SocketAddress &addr, int queuelength, int &error);
    int Accept(SocketAddress *addr);

};
#endif
