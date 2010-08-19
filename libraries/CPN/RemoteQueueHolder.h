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
 * \brief An object to hold references to RemoteQueues so they can
 * continue to work after the node has gone away.
 * \author John Bridgman
 */
#ifndef CPN_REMOTEQUEUEHOLDER_H
#define CPN_REMOTEQUEUEHOLDER_H
#pragma once
#include "CPNCommon.h"
#include "WakeupHandle.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include <map>
#include <vector>
namespace CPN {
    class RemoteQueueHolder {
    public:
        typedef std::map<Key_t, shared_ptr<RemoteQueue> > QueueMap;
        typedef std::vector<shared_ptr<RemoteQueue> > QueueList;
        void AddQueue(shared_ptr<RemoteQueue> queue);
        void CleanupQueue(Key_t key);
        void Cleanup();
        void Shutdown();
        WakeupHandle *GetWakeup() { return &wakeup; }
    private:
        void PrintState();
        QueueMap queuemap;
        QueueList queuelist;
        PthreadMutex lock;
        PthreadCondition cond;
        WakeupHandle wakeup;
    };
}
#endif
