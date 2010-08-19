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
#include <cassert>

namespace CPN {

    typedef AutoLock<PthreadMutex> AutoPLock;

    ConnectionServer::ConnectionServer(SockAddrList addrs, shared_ptr<Database> db)
        : database(db), logger(db.get(), Logger::INFO), enabled(true)
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
                    shared_ptr<PendingConnection> conn;
                    std::pair<PendingMap::iterator, PendingMap::iterator> range;
                    range = pendingconnections.equal_range(key);
                    PendingMap::iterator entry = range.first;
                    PendingMap::iterator end = range.second;
                    while (entry != end) {
                        if (!entry->second->Done()) {
                            conn = entry->second;
                            break;
                        }
                        ++entry;
                    }
                    if (!conn) {
                        conn = shared_ptr<PendingConnection>(new PendingConnection(key, this));
                        pendingconnections.insert(std::make_pair(key, conn));
                    }
                    conn->Set(sock.FD());
                    sock.Reset();
                }
            } catch (const ErrnoException &e) {
                // Ignore, if we had an error we closed the socket when we
                // left the scope, will try again later
                logger.Error("Exception in ConnectionServer main loop (e: %d): %s", e.Error(), e.what());
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
        AutoPLock al(lock);
        shared_ptr<PendingConnection> conn;
        if (enabled) {
            al.Unlock();
            conn = shared_ptr<PendingConnection>(new PendingConnection(writerkey, this));
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
                conn->Cancel();
            } else {
                conn->Set(sock.FD());
                sock.Reset();
            }
        } else {
            conn = shared_ptr<PendingConnection>(new PendingConnection(writerkey, this));
            pendingconnections.insert(std::make_pair(writerkey, conn));
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

    void ConnectionServer::Disable() {
        AutoPLock al(lock);
        enabled = false;
    }

    void ConnectionServer::Enable() {
        AutoPLock al(lock);
        enabled = true;
        PendingMap::iterator entry = pendingconnections.begin();
        while (entry != pendingconnections.end()) {
            entry->second->Cancel();
            ++entry;
        }
    }

    void ConnectionServer::LogState() {
        if (server.Closed()) {
            logger.Error("Server socket closed");
        }
        if (!enabled) {
            logger.Error("Connection handler disabled??");
        }
        logger.Error("%u pending connections", pendingconnections.size());
    }

    void ConnectionServer::PendingDone(Key_t key, PendingConnection *conn) {
        AutoPLock al(lock);
        std::pair<PendingMap::iterator, PendingMap::iterator> range;
        range = pendingconnections.equal_range(key);
        PendingMap::iterator entry = range.first;
        PendingMap::iterator end = range.second;
        while (entry != end) {
            if (entry->second.get() == conn) {
                pendingconnections.erase(entry);
                break;
            }
            ++entry;
        }
    }

    ConnectionServer::PendingConnection::PendingConnection(Key_t k, ConnectionServer *serv)
        : key(k), server(serv)
    {}

    ConnectionServer::PendingConnection::~PendingConnection() {
    }

    int ConnectionServer::PendingConnection::Get() {
        AutoPLock al(future_lock);
        InternalWait();
        ret = -1;
        al.Unlock();
        AutoPLock alf(file_lock);
        int filed = fd;
        fd = -1;
        alf.Unlock();
        server->PendingDone(key, this);
        return filed;
    }

    void ConnectionServer::PendingConnection::Set(int filed) {
        AutoPLock alf(file_lock);
        fd = filed;
        alf.Unlock();
        AutoPLock al(future_lock);
        ASSERT(!done);
        InternalSet(filed);
    }

}
