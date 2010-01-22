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

#include "RemoteDatabaseDaemon.h"
#include "Assert.h"
#include "ErrnoException.h"
#include <stdio.h>

RemoteDatabaseDaemon::RemoteDatabaseDaemon(const SocketAddress &addr)
{
    Listen(addr);
}

RemoteDatabaseDaemon::RemoteDatabaseDaemon(const SockAddrList &addrs)
{
    Listen(addrs);
}
RemoteDatabaseDaemon::~RemoteDatabaseDaemon() {
}

void RemoteDatabaseDaemon::Run() {
    Readable(true);
    SocketAddress addr;
    addr.SetFromSockName(FD());
    dbprintf(1, "Listening on %s:%s\n", addr.GetHostName().c_str(), addr.GetServName().c_str());
    while (true) {
        ClientMap::iterator itr = clients.begin();
        std::vector<FileHandler*> fds;
        if (!Closed()) {
            fds.push_back(this);
        }
        while (itr != clients.end()) {
            ClientPtr client = itr->second;
            if (client->Closed()) {
                ClientMap::iterator todelete = itr;
                ++itr;
                clients.erase(todelete);
            } else {
                fds.push_back(client.get());
                ++itr;
            }
        }
        if ((clients.empty() && IsTerminated()) || fds.empty()) {
            break;
        }
        Poll(&fds[0], fds.size(), -1);
    }
}

void RemoteDatabaseDaemon::Terminate() {
    CPN::RemoteDBServer::Terminate();
    Close();
}

void RemoteDatabaseDaemon::Terminate(const std::string &name) {
    dbprintf(1, "Terminating %s\n", name.c_str());
    clients[name]->Close();
}

void RemoteDatabaseDaemon::OnRead() {
    int nfd = Accept();
    if (nfd >= 0) {
        ClientPtr conn = ClientPtr(new Client(this, nfd));
        clients.insert(std::make_pair(conn->GetName(), conn));
    }
}

void RemoteDatabaseDaemon::OnError() {
    Terminate();
}

void RemoteDatabaseDaemon::OnInval() {
    Terminate();
}

void RemoteDatabaseDaemon::SendMessage(const std::string &recipient, const Variant &msg) {
    ClientMap::iterator entry = clients.find(recipient);
    ASSERT(entry != clients.end());
    dbprintf(4, "reply:%s:%s\n", recipient.c_str(), msg.AsJSON().c_str());
    entry->second->Send(msg);
}

void RemoteDatabaseDaemon::BroadcastMessage(const Variant &msg) {
    ClientMap::iterator entry = clients.begin();
    dbprintf(4, "broadcast:%s\n", msg.AsJSON().c_str());
    while (entry != clients.end()) {
        entry->second->Send(msg);
        ++entry;
    }
}

void RemoteDatabaseDaemon::LogMessage(const std::string &msg) {
    dbprintf(1, "log:%s\n", msg.c_str());
}

RemoteDatabaseDaemon::Client::Client(RemoteDatabaseDaemon *d, int nfd)
    : SockHandler(nfd), daemon(d)
{
    Readable(true);
    SocketAddress addr;
    addr.SetFromPeerName(FD());
    name = addr.GetHostName() + ":" + addr.GetServName();
    d->dbprintf(1, "New connection from %s\n", name.c_str());
}

void RemoteDatabaseDaemon::Client::OnRead() {
    const unsigned BUF_SIZE = 4*1024;
    char buf[BUF_SIZE];
    try {
        while (Good()) {
            unsigned numread = Recv(buf, BUF_SIZE, false);
            if (numread == 0) {
                if (Eof()) {
                    daemon->Terminate(name);
                }
                break;
            }
            for (unsigned i = 0; i < numread; ++i) {
                if (buf[i] == 0) {
                    Variant msg = Variant::FromJSON(buffer);
                    daemon->DispatchMessage(name, msg);
                    buffer.clear();
                } else {
                    buffer.push_back(buf[i]);
                }
            }
        }
    } catch (const ErrnoException &e) {
        daemon->dbprintf(0, "Error on read from %s:%d: %s\n", name.c_str(), e.Error(), e.what());
    }
}

void RemoteDatabaseDaemon::Client::OnWrite() {
}

void RemoteDatabaseDaemon::Client::OnError() {
    daemon->Terminate(name);
}

void RemoteDatabaseDaemon::Client::OnHup() {
    daemon->Terminate(name);
}

void RemoteDatabaseDaemon::Client::OnInval() {
    daemon->Terminate(name);
}

void RemoteDatabaseDaemon::Client::Send(const Variant &msg) {
    try {
        std::string message = msg.AsJSON();
        Write(message.c_str(), message.size() + 1);
    } catch (const ErrnoException &e) {
        daemon->dbprintf(0, "Error on write to %s:%d: %s\n", name.c_str(), e.Error(), e.what());
        daemon->Terminate(name);
    }
}


