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
#include "PthreadCondition.h"
#include <map>
namespace CPN {
    class ConnectionServer {
    public:
        ConnectionServer(SockAddrList addrs, shared_ptr<Database> db);
        void Poll();
        void Wakeup() { wakeup.SendWakeup(); }
        void Close();
        shared_ptr<Future<int> > ConnectWriter(Key_t writerkey);
        shared_ptr<Future<int> > ConnectReader(Key_t readerkey);
        SocketAddress GetAddress();

        // For testing
        void Disable();
        void Enable();

        /// For debug ONLY!
        void LogState();
    private:
        class PendingConnection : public Future<int>, public SocketHandle {
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
        shared_ptr<Database> database;
        Logger logger;
        ServerSocketHandle server;
        WakeupHandle wakeup;
        PendingMap pendingconnections;
        bool enabled;
    };
}
#endif
