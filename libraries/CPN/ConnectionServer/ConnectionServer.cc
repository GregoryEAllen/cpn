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
#include "ConnectionServer.h"
#include "PacketHeader.h"
#include "Database.h"
#include "AutoLock.h"
#include "Assert.h"
#include "ErrnoException.h"
#include <deque>

namespace CPN {

    typedef Sync::AutoLock<PthreadMutex> AutoPLock;

    ConnectionServer::ConnectionServer(SockAddrList addrs, shared_ptr<Database> db)
        : database(db)
    {
        server.Listen(addrs);
    }

    void ConnectionServer::Poll() {
        std::deque<FileHandle*> files;
        files.push_back(&server);
        files.push_back(&wakeup);
        FileHandle::Poll(files.begin(), files.end(), -1);
        wakeup.Read();
        if (server.Readable()) {
            server.Readable(false);
            try {
                SocketHandle sock(server.Accept());
                Packet packet;
                unsigned num = sock.Read(&packet.header, sizeof(packet.header));
                if (num != sizeof(packet.header)) {
                    return;
                }
                if (!packet.Valid()) {
                    return;
                }
                if ((packet.Type() == PACKET_ID_READER)
                        || (packet.Type() == PACKET_ID_WRITER)) {
                    AutoPLock al(lock);
                    Key_t key = packet.DestinationKey();
                    PendingMap::iterator entry = pendingconnections.find(key);
                    if (entry == pendingconnections.end()) {
                        shared_ptr<PendingConnection> conn
                            = shared_ptr<PendingConnection>(new PendingConnection(key, this));
                        conn->Set(sock.FD());
                        sock.Reset();
                        pendingconnections.insert(std::make_pair(key, conn));
                    } else {
                        entry->second->Set(sock.FD());
                        sock.Reset();
                    }
                }
            } catch (const ErrnoException &e) {
                // Ignore, if we had an error we closed the socket when we
                // left the scope, will try again later
            }
        }
    }

    void ConnectionServer::Close() {
        AutoPLock al(lock);
        server.Close();
        for (PendingMap::iterator itr = pendingconnections.begin();
                itr != pendingconnections.end(); ++itr)
        {
            itr->second->Cancel();
        }
    }

    shared_ptr<Future<int> > ConnectionServer::ConnectWriter(Key_t writerkey) {
        if (server.Closed()) {
            return shared_ptr<Future<int> >();
        }
        shared_ptr<PendingConnection> conn = 
            shared_ptr<PendingConnection>(new PendingConnection(writerkey, this));
        Key_t readerkey = database->GetWritersReader(writerkey);
        Key_t hostkey = database->GetReaderHost(readerkey);
        std::string hostname;
        std::string servname;
        database->GetHostConnectionInfo(hostkey, hostname, servname);
        SocketHandle sock;
        sock.Connect(SocketAddress::CreateIP(hostname, servname));
        Packet packet(PACKET_ID_WRITER);
        packet.SourceKey(writerkey).DestinationKey(readerkey);
        unsigned num = sock.Write(&packet.header, sizeof(packet.header));
        if (num != sizeof(packet.header)) {
            conn->Set(-1);
        } else {
            conn->Set(sock.FD());
            sock.Reset();
        }
        return conn;
    }

    shared_ptr<Future<int> > ConnectionServer::ConnectReader(Key_t readerkey) {
        if (server.Closed()) {
            return shared_ptr<Future<int> >();
        }
        AutoPLock al(lock);
        shared_ptr<PendingConnection> conn;
        PendingMap::iterator entry = pendingconnections.find(readerkey);
        if (entry == pendingconnections.end()) {
            conn = shared_ptr<PendingConnection>(new PendingConnection(readerkey, this));
            pendingconnections.insert(std::make_pair(readerkey, conn));
        } else {
            conn = entry->second;
        }
        return conn;
    }

    SocketAddress ConnectionServer::GetAddress() {
        SocketAddress addr;
        addr.SetFromSockName(server.FD());
        return addr;
    }

    void ConnectionServer::PendingDone(Key_t key) {
        AutoPLock al(lock);
        PendingMap::iterator entry = pendingconnections.find(key);
        if (entry != pendingconnections.end()) {
            pendingconnections.erase(entry);
        }
    }

    ConnectionServer::PendingConnection::PendingConnection(Key_t k, ConnectionServer *serv)
        : key(k), fd(-1), done(false), server(serv)
    {}

    int ConnectionServer::PendingConnection::Get() {
        AutoPLock al(lock);
        while (!done) {
            cond.Wait(lock);
        }
        int filed = fd;
        fd = -1;
        al.Unlock();
        server->PendingDone(key);
        return filed;
    }

    void ConnectionServer::PendingConnection::Set(int filed) {
        AutoPLock al(lock);
        ASSERT(!done);
        fd = filed;
        done = true;
        cond.Signal();
    }

    bool ConnectionServer::PendingConnection::Done() {
        AutoPLock al(lock);
        return done;
    }

    void ConnectionServer::PendingConnection::Cancel() {
        AutoPLock al(lock);
        if (!done) {
            fd = -1;
            done = true;
        }
    }
}
