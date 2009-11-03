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
#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H
#pragma once
#include "FileHandler.h"
#include "SocketAddress.h"

class SockHandler : public FileHandler {
public:

    static void CreatePair(SockHandler &sock1, SockHandler &sock2);
    static void CreatePair(int fd[2]);

    void Connect(const SocketAddress &addr);

    void Connect(const SockAddrList &addrs);

    void ShutdownRead();
    void ShutdownWrite();
    // If block true then wait for buffer to be full
    // if block false don't wait for anything
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
        SendOpts &Block(bool block);
        SendOpts &NoSignal(bool sig);
        SendOpts &More(bool more);
        int flags;
    };

    unsigned Send(const void *ptr, unsigned len, const SendOpts &opts);
private:
    bool Connect(const SocketAddress &addr, int &error);
};
#endif
