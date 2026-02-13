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
#ifndef CPN_CONNECTIONSERVER_H
#define CPN_CONNECTIONSERVER_H
#pragma once
#include "CPNCommon.h"
#include "ServerSocketHandle.h"
#include "SocketHandle.h"
#include "WakeupHandle.h"
#include "Logger.h"
#include "Future.h"
#include "PthreadMutex.h"
#include <map>
namespace CPN {
    /**
     * The connnection server takes ownership of the listening socket and accepts
     * new connections. It also has all the logic for creating new connections.
     * When remote is active in the Kernel, the Kernel creates a connection server
     * and all RemoteQueues receive a reference to it as well.
     */
    class CPN_LOCAL ConnectionServer {
    public:
        /**
         * \param addrs list of socket addresses to use to listen on
         * \param ctx the context which contains all the data connection data.
         */
        ConnectionServer(SockAddrList addrs, shared_ptr<Context> ctx);
        /**
         * Poll is periodically called by the Kernel to accept new connections.
         */
        void Poll();
        /**
         * A call to Wakeup will cause Poll to return even if no connections
         * are accepted.
         */
        void Wakeup() { wakeup.SendWakeup(); }
        /**
         * Close the listening socket and shutdown the connection server.
         */
        void Close();
        /**
         * Request the connectino server to connect the RemoteQueue which has
         * key writekey to its other endpoint.
         * \param writerkey the writekey of the endpoint.
         * \return A Sync::Future which will contain the file descriptor
         * when finished
         */
        shared_ptr<Sync::Future<int> > ConnectWriter(Key_t writerkey);
        /**
         * Request the connection server connect the RemoteQueue which has
         * key readerkey to its other endpoint.
         * \param readerkey the key for the reader of this queue
         * \return A Sync::Future which will have the file descriptor.
         */
        shared_ptr<Sync::Future<int> > ConnectReader(Key_t readerkey);
        /**
         * \return the address this connection server is listening on
         */
        SocketAddress GetAddress();

        /**
         * These functions are for testing and debugging.
         * They should only be called from the unit tests or from
         * the debugger.
         * @{
         */
        void Disable();
        void Enable();
        void LogState();
        /** @} */
    private:
        class PendingConnection : public Sync::Future<int>, public SocketHandle {
        public:
            PendingConnection(Key_t k, ConnectionServer *serv);
            ~PendingConnection();
            int Get();
            void Set(int filed);
            Key_t GetKey() const { return key; }
        private:
            const Key_t key;
            ConnectionServer *server;
        };

        void PendingDone(Key_t key, PendingConnection *conn);

        typedef std::multimap<Key_t, shared_ptr<PendingConnection> > PendingMap;
        PthreadMutex lock;
        shared_ptr<Context> context;
        Logger logger;
        ServerSocketHandle server;
        WakeupHandle wakeup;
        PendingMap pendingconnections;
        bool enabled;
    };
}
#endif
