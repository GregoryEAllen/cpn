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

#include "RemoteContextDaemon.h"
#include "ThrowingAssert.h"
#include "ErrnoException.h"
#include "VariantToJSON.h"
#include <stdio.h>

using std::auto_ptr;

RemoteContextDaemon::RemoteContextDaemon(const SocketAddress &addr)
{
    Listen(addr);
    SetReuseAddr();
}

RemoteContextDaemon::RemoteContextDaemon(const SockAddrList &addrs)
{
    Listen(addrs);
    SetReuseAddr();
}
RemoteContextDaemon::~RemoteContextDaemon() {
}

void RemoteContextDaemon::Run() {
    Readable(false);
    SocketAddress addr;
    addr.SetFromSockName(FD());
    dbprintf(1, "Listening on %s:%s\n", addr.GetHostName().c_str(), addr.GetServName().c_str());
    while (true) {
        std::vector<FileHandle*> fds;
        if (!Closed()) {
            if (Readable()) {
                Read();
            }
            fds.push_back(this);
        }
        ClientMap::iterator itr = clients.begin();
        while (itr != clients.end()) {
            ClientPtr client = itr->second;
            if (client->Closed()) {
                ClientMap::iterator todelete = itr;
                ++itr;
                clients.erase(todelete);
                Terminate();
            } else {
                if (client->Readable()) {
                    client->Read();
                }
                fds.push_back(client.get());
                ++itr;
            }
        }
        if ((clients.empty() && IsTerminated()) || fds.empty()) {
            break;
        }
        Poll(fds.begin(), fds.end(), -1);
    }
}

void RemoteContextDaemon::Terminate() {
    if (IsTerminated()) return;
    CPN::RemoteContextServer::Terminate();
    Close();
    ClientMap::iterator itr = clients.begin();
    while (itr != clients.end()) {
        ClientPtr client = itr->second;
        if (!client->Closed()) {
            try {
                client->ShutdownWrite();
            } catch (const ErrnoException &e) {
                //ignore
            }
        }
        ++itr;
    }
}

void RemoteContextDaemon::Terminate(const std::string &name) {
    dbprintf(1, "Terminating %s\n", name.c_str());
    clients[name]->Close();
}

void RemoteContextDaemon::Read() {
    int nfd = Accept();
    if (nfd >= 0) {
        ClientPtr conn = ClientPtr(new Client(this, nfd));
        clients.insert(std::make_pair(conn->GetName(), conn));
        Readable(false);
    }
}

void RemoteContextDaemon::SendMessage(const std::string &recipient, const Variant &msg) {
    if (IsTerminated()) {
        dbprintf(1, "Trying to send a reply after shutdown\n%s:%s",
                recipient.c_str(), VariantToJSON(msg).c_str());
        return;
    }
    ClientMap::iterator entry = clients.find(recipient);
    ASSERT(entry != clients.end());
    dbprintf(4, "reply:%s:%s\n", recipient.c_str(), VariantToJSON(msg).c_str());
    entry->second->Send(msg);
}

void RemoteContextDaemon::BroadcastMessage(const Variant &msg) {
    if (IsTerminated()) {
        dbprintf(1, "Trying to braodcast after shutdown\n%s", VariantToJSON(msg).c_str());
        return;
    }
    ClientMap::iterator entry = clients.begin();
    dbprintf(4, "broadcast:%s\n", VariantToJSON(msg).c_str());
    while (entry != clients.end()) {
        entry->second->Send(msg);
        ++entry;
    }
}

void RemoteContextDaemon::LogMessage(const std::string &msg) {
    dbprintf(1, "log:%s\n", msg.c_str());
}

RemoteContextDaemon::Client::Client(RemoteContextDaemon *d, int nfd)
    : SocketHandle(nfd), daemon(d)
{
    Readable(false);
    SetNoDelay(true);
    SocketAddress addr;
    addr.SetFromPeerName(FD());
    name = addr.GetHostName() + ":" + addr.GetServName();
    d->dbprintf(1, "New connection from %s\n", name.c_str());
}

void RemoteContextDaemon::Client::Read() {
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
            unsigned numparsed = 0;
            while (numparsed < numread) {
                numparsed += parse.Parse(buf + numparsed, numread - numparsed);
                if (parse.Done()) {
                    daemon->DispatchMessage(name, parse.Get());
                    parse.Reset();
                } else if (parse.Error()) {
                    daemon->dbprintf(0, "Error parsing input!\n");
                    parse.Reset();
                }
            }
        }
    } catch (const ErrnoException &e) {
        daemon->dbprintf(0, "Error on read from %s:%d: %s\n", name.c_str(), e.Error(), e.what());
    }
}

void RemoteContextDaemon::Client::Send(const Variant &msg) {
    if (Closed()) {
        return;
    }
    try {
        std::string message = VariantToJSON(msg);
        unsigned numwritten = 0;
        while (numwritten < message.size()) {
            numwritten += Write(message.data() + numwritten, message.size() - numwritten);
        }
    } catch (const ErrnoException &e) {
        daemon->dbprintf(0, "Error on write to %s:%d: %s\n", name.c_str(), e.Error(), e.what());
        daemon->Terminate(name);
    }
}


