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
#ifndef REMOTEDATABASE_H
#define REMOTEDATABASE_H
#pragma once
#include "RemoteDBClient.h"
#include "SocketHandle.h"
#include "Pthread.h"
#include <vector>

/**
 * An implementation for the RemoteDBClient that is paired with RemoteDatabaseDaemon.
 */
class RemoteDatabase : public Pthread, public CPN::RemoteDBClient, public SocketHandle {
public:
    RemoteDatabase(const SocketAddress &addr) { Connect(addr); }
    RemoteDatabase(const SockAddrList &addrs) { Connect(addrs); }
    ~RemoteDatabase();
protected:
    void SendMessage(const Variant &msg);
    void *EntryPoint();
    void Read();
private:
    std::vector<char> buffer;
};
#endif
