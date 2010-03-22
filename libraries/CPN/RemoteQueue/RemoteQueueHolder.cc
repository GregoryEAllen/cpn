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
#include "RemoteQueueHolder.h"
#include "RemoteQueue.h"
#include "AutoLock.h"
#include <stdio.h>

namespace CPN {

    void RemoteQueueHolder::AddQueue(shared_ptr<RemoteQueue> queue) {
        Sync::AutoLock<PthreadMutex> al(lock);
        queuemap.insert(std::make_pair(queue->GetKey(), queue));
    }

    void RemoteQueueHolder::CleanupQueue(Key_t key) {
        Sync::AutoLock<PthreadMutex> al(lock);
        QueueMap::iterator entry = queuemap.find(key);
        if (entry != queuemap.end()) {
            queuelist.push_back(entry->second);
            queuemap.erase(entry);
            /*
            printf("CleanupQueue %llu\n", (unsigned long long)pthread_self());
        } else {
            printf("CleanupQueue %llu, already gone!?\n", (unsigned long long)pthread_self());
            */
        }
        cond.Signal();
    }

    void RemoteQueueHolder::Cleanup() {
        Sync::AutoLock<PthreadMutex> al(lock);
        QueueList tmp;
        tmp.swap(queuelist);
        al.Unlock();
        tmp.clear();
    }

    void RemoteQueueHolder::Shutdown() {
        Sync::AutoLock<PthreadMutex> al(lock);
        QueueMap::iterator q = queuemap.begin();
        while (q != queuemap.end()) {
            /*
            if (!q->second->Running()) {
                QueueMap::iterator entry = q;
                ++q;
                entry->second->LogState();
                queuemap.erase(entry);
            } else {
            */
                q->second->Shutdown();
                ++q;
            //}
        }
        wakeup.SendWakeup();
        while (!queuemap.empty()) {
            cond.Wait(lock);
        }
        queuelist.clear();
    }

    void RemoteQueueHolder::PrintState() {
        QueueMap::iterator q = queuemap.begin();
        while (q != queuemap.end()) {
            q->second->LogState();
            ++q;
        }
        QueueList::iterator itr = queuelist.begin();
        while (itr != queuelist.end()) {
            itr->get()->LogState();
            ++itr;
        }
    }
}

