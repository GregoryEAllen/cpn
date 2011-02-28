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
#ifndef REMOTECONTEXTDAEMON_H
#define REMOTECONTEXTDAEMON_H
#pragma once
#include "ServerSocketHandle.h"
#include "SocketHandle.h"
#include "RemoteContextServer.h"
#include "JSONToVariant.h"
#include <string>
#include <tr1/memory>
#include <memory>

/**
 * The RemoteContextDaemon is an implementation of RemoteContextServer that uses
 * simple tcp/ip and listens on an address you specify.
 */
class CPN_API RemoteContextDaemon : public CPN::RemoteContextServer, public ServerSocketHandle {
public:
    RemoteContextDaemon(const SocketAddress &addr);
    RemoteContextDaemon(const SockAddrList &addrs);
    ~RemoteContextDaemon();
    /** \brief Run the actual Context
     * returns when the context is terminated
     * and all clients have disconnected
     */
    void Run();

    // These functions are for the Client
    using CPN::RemoteContextServer::DispatchMessage;
    void Terminate();
    void Terminate(const std::string &name);
private:
    void Read();
    void SendMessage(const std::string &recipient, const Variant &msg);
    void BroadcastMessage(const Variant &msg);
    void LogMessage(const std::string &msg);

    class Client : public SocketHandle {
    public:
        Client(RemoteContextDaemon *d, int nfd);
        void Read();
        void Send(const Variant &msg);
        const std::string &GetName() const { return name; }
    private:
        RemoteContextDaemon *daemon;
        std::string name;
        JSONToVariant parse;
    };
    typedef std::tr1::shared_ptr<Client> ClientPtr;
    typedef std::map<std::string, ClientPtr> ClientMap;

    ClientMap clients;
};

#endif
