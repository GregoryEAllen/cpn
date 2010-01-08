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
#include "ListenSockHandler.h"
#include "SockHandler.h"
#include "RemoteDBServer.h"
#include <list>
#include <vector>
#include <string>
#include <tr1/memory>

class RemoteDatabaseDaemon : public CPN::RemoteDBServer, private ListenSockHandler {
public:
    RemoteDatabaseDaemon(const SocketAddress &addr);
    RemoteDatabaseDaemon(const SockAddrList &addrs);
    ~RemoteDatabaseDaemon();
    void Run();
    using CPN::RemoteDBServer::DispatchMessage;
    void Terminate();
private:
    void OnRead();
    void OnError();
    void OnInval();
    void SendMessage(const std::string &recipient, const Variant &msg);
    void BroadcastMessage(const Variant &msg);

    class Client : public SockHandler {
    public:
        Client(RemoteDatabaseDaemon *d, int nfd);
        void OnRead();
        void OnWrite();
        void OnError();
        void OnHup();
        void OnInval();
        void Send(const Variant &msg);
        const std::string &GetName() const { return name; }
    private:
        RemoteDatabaseDaemon *daemon;
        std::string name;
        std::vector<char> buffer;
    };
    typedef std::tr1::shared_ptr<Client> ClientPtr;
    typedef std::map<std::string, ClientPtr> ClientMap;

    ClientMap clients;
};

#endif
