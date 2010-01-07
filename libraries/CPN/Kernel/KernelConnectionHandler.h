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

#ifndef CPN_KERNELCONNECTIONHANDLER_H
#define CPN_KERNELCONNECTIONHANDLER_H
#pragma once

#include "CPNCommon.h"
#include "KernelBase.h"

#include "ListenSockHandler.h"
#include "Future.h"
#include "ReentrantLock.h"
#include "Logger.h"
#include <list>
#include <map>

namespace CPN {
    class KernelConnectionHandler : public ListenSockHandler {
    public:
        KernelConnectionHandler(KernelBase *kmh_);
        void OnRead();
        void OnError();
        void OnInval();
        void Register(std::vector<FileHandler*> &filehandlers);
        void Shutdown();
        shared_ptr<Future<int> > GetReaderDescriptor(Key_t readerkey, Key_t writerkey);
        shared_ptr<Future<int> > GetWriterDescriptor(Key_t readerkey, Key_t writerkey);
        void SetupLogger();

    private:
        void LogState();

        Sync::ReentrantLock lock;
        Logger logger;
        KernelBase *kmh;
        class Connection;
        void Transfer(Key_t key, shared_ptr<Connection> conn);

        typedef std::list<shared_ptr<Connection> > ConnList;
        ConnList connlist;
        typedef std::map<Key_t, shared_ptr<Connection> > ConnMap;
        ConnMap connmap;
    };
}

#endif
