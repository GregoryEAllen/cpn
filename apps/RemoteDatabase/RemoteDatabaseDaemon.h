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
#ifndef REMOTEDATABASEDAEMON_H
#define REMOTEDATABASEDAEMON_H
#pragma once
#include "ServerSocketHandle.h"
#include "SocketHandle.h"
#include "RemoteDBServer.h"
#include "JSONToVariant.h"
#include <list>
#include <vector>
#include <string>
#include <tr1/memory>
#include <memory>

/**
 * the RemoteDatabaseDaemon is an implementation of RemoteDBServer that uses
 * simple tcp/ip and listens on an address you specify.
 */
class RemoteDatabaseDaemon : public CPN::RemoteDBServer, public ServerSocketHandle {
public:
    RemoteDatabaseDaemon(const SocketAddress &addr);
    RemoteDatabaseDaemon(const SockAddrList &addrs);
    ~RemoteDatabaseDaemon();
    /** \brief Run the actual database
     * returns when the database is terminated
     * and all clients have disconnected
     */
    void Run();

    // These functions are for the Client
    using CPN::RemoteDBServer::DispatchMessage;
    void Terminate();
    void Terminate(const std::string &name);
private:
    void Read();
    void SendMessage(const std::string &recipient, const Variant &msg);
    void BroadcastMessage(const Variant &msg);
    void LogMessage(const std::string &msg);

    class Client : public SocketHandle {
    public:
        Client(RemoteDatabaseDaemon *d, int nfd);
        void Read();
        void Send(const Variant &msg);
        const std::string &GetName() const { return name; }
    private:
        RemoteDatabaseDaemon *daemon;
        std::string name;
        JSONToVariant parse;
    };
    typedef std::tr1::shared_ptr<Client> ClientPtr;
    typedef std::map<std::string, ClientPtr> ClientMap;

    ClientMap clients;
};

#endif
