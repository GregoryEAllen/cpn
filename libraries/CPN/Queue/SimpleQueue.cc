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

#include "SimpleQueue.h"
#include "QueueAttr.h"

namespace CPN {

    SimpleQueue::SimpleQueue(shared_ptr<Database> db, const SimpleQueueAttr &attr)
        : QueueBase(db, attr),
        queue(0),
        oldqueue(0),
        enqueueUseOld(false),
        dequeueUseOld(false)
    {
        queue = new CircularQueue(attr.GetLength(), attr.GetMaxThreshold(), attr.GetNumChannels());
    }

    SimpleQueue::~SimpleQueue() {
        delete queue;
        queue = 0;
        delete oldqueue;
        oldqueue = 0;
    }

    void *SimpleQueue::InternalGetRawEnqueuePtr(unsigned thresh, unsigned chan) {
        if (enqueueUseOld) {
            return oldqueue->GetRawEnqueuePtr(thresh, chan);
        } else {
            return queue->GetRawEnqueuePtr(thresh, chan);
        }
    }

    void SimpleQueue::InternalEnqueue(unsigned count) {
        if (enqueueUseOld) {
            oldqueue->Dequeue(oldqueue->Count());
            oldqueue->Enqueue(count);
            const void *ptr = oldqueue->GetRawDequeuePtr(count, 0);
            queue->RawEnqueue(ptr, count, oldqueue->NumChannels(), oldqueue->ChannelStride());
            enqueueUseOld = false;
            delete oldqueue;
            oldqueue = 0;
        } else {
            queue->Enqueue(count);
        }
    }

    const void *SimpleQueue::InternalGetRawDequeuePtr(unsigned thresh, unsigned chan) {
        if (dequeueUseOld) {
            return oldqueue->GetRawDequeuePtr(thresh, chan);
        } else {
            return queue->GetRawDequeuePtr(thresh, chan);
        }
    }

    void SimpleQueue::InternalDequeue(unsigned count) {
        if (dequeueUseOld) {
            dequeueUseOld = false;
            delete oldqueue;
            oldqueue = 0;
        }
        queue->Dequeue(count);
    }

    unsigned SimpleQueue::NumChannels() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue->NumChannels();
    }

    unsigned SimpleQueue::MaxThreshold() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue->MaxThreshold();
    }

    unsigned SimpleQueue::QueueLength() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue->QueueLength();
    }

    unsigned SimpleQueue::Freespace() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue->Freespace();
    }

    bool SimpleQueue::Full() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue->Full();
    }

    unsigned SimpleQueue::Count() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue->Count();
    }

    bool SimpleQueue::Empty() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue->Empty();
    }

    unsigned SimpleQueue::ChannelStride() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue->ChannelStride();
    }

    void SimpleQueue::Grow(unsigned queueLen, unsigned maxThresh) {
        Sync::AutoLock<QueueBase> al(*this);
        ASSERT(!(inenqueue && indequeue), "Unhandled grow case of having an outstanding dequeue and enqueue");
        if (oldqueue) {
            ASSERT(enqueueUseOld == inenqueue);
            ASSERT(dequeueUseOld == indequeue);
            queue->Grow(queueLen, maxThresh);
        } else {
            enqueueUseOld = inenqueue;
            dequeueUseOld = indequeue;
            oldqueue = queue;
            // this should make a duplicate
            // For CircularQueue this makes an actual copy
            queue = new CircularQueue(*oldqueue);
            queue->Grow(queueLen, maxThresh);
        }
    }

}

