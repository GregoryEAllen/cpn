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
    /**
     * RemoteQueueHolder takes responsibility of holding references to the
     * remote queues so that they can exist after the node has terminated.
     * This is required because a node can terminate and the RemoteQueue has
     * not finished sending all its data to the other side.
     * This class also allows us to signal to the remote queues to shut down.
     * Note that because closing a socket while selecting on that socket is a race
     * condition and the only other good way of breaking out of the select is to
     * also select on some other file, this holder provides a FileHandle that
     * will cause select to return when shutting down.
     */
    class RemoteQueueHolder {
    public:
        typedef std::map<Key_t, shared_ptr<RemoteQueue> > QueueMap;
        typedef std::vector<shared_ptr<RemoteQueue> > QueueList;
        /**
         * Add a queue.
         * \param queue the queue to add
         */
        void AddQueue(shared_ptr<RemoteQueue> queue);
        /**
         * Called by the RemoteQueue it self to say it has completed.
         * \param key the endpoint key for the queue
         */
        void CleanupQueue(Key_t key);
        /**
         * Called by the kernel periodically to clean up
         * queues that are done.
         */
        void Cleanup();
        /**
         * Call shutdown on each queue and cause the WakeupHandle to
         * make select return.
         */
        void Shutdown();
        /**
         * The WakeupHandle is used by all the RemoteQueues to break
         * out of select when shutting down.
         * \return A WakeupHandle pointer.
         */
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
