//=============================================================================
//  Computational Process Networks class library
//  Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//  This library is free software; you can redistribute it and/or modify it
//  under the terms of the GNU Library General Public License as published
//  by the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Library General Public License for more details.
//
//  The GNU Public License is available in the file LICENSE, or you
//  can write to the Free Software Foundation, Inc., 59 Temple Place -
//  Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//  World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \brief Implementation of CPN ThresholdQueue
 * \author John Bridgman
 */

#include "ThresholdQueue.h"
#include "QueueAttr.h"
#include "Assert.h"
#include <cstring>

namespace CPN {

    ThresholdQueue::ThresholdQueue(shared_ptr<Database> db, const SimpleQueueAttr &attr)
        : QueueBase(db, attr), queue(0), oldqueue(0), enqueueUseOld(false), dequeueUseOld(false)
    {
        ThresholdQueueAttr qattr(attr.GetLength(), attr.GetMaxThreshold(),
                attr.GetNumChannels(), attr.GetHint() == QUEUEHINT_THRESHOLD);
        queue = new TQImpl(qattr);
    }

    ThresholdQueue::ThresholdQueue(shared_ptr<Database> db, const SimpleQueueAttr &attr,
            unsigned length)
        : QueueBase(db, attr), queue(0), oldqueue(0), enqueueUseOld(false), dequeueUseOld(false)
    {
        ThresholdQueueAttr qattr(length, attr.GetMaxThreshold(),
                attr.GetNumChannels(), attr.GetHint() == QUEUEHINT_THRESHOLD);
        queue = new TQImpl(qattr);
    }


    ThresholdQueue::~ThresholdQueue() {
        delete queue;
        queue = 0;
        delete oldqueue;
        oldqueue = 0;
    }

    void *ThresholdQueue::InternalGetRawEnqueuePtr(unsigned thresh, unsigned chan) {
        void *ret = 0;
        if (enqueueUseOld) {
            ASSERT(inenqueue);
            ret = oldqueue->GetRawEnqueuePtr(thresh, chan);
            ASSERT(ret);
        } else {
            ret = queue->GetRawEnqueuePtr(thresh, chan);
        }
        return ret;
    }

    void ThresholdQueue::InternalEnqueue(unsigned count) {
        if (enqueueUseOld) {
            oldqueue->Dequeue(oldqueue->Count());
            oldqueue->Enqueue(count);

            unsigned count;
            while ( (count = oldqueue->Count()) != 0 ) {
                if (count > oldqueue->MaxThreshold()) {
                    count = oldqueue->MaxThreshold();
                }
                for (unsigned chan = 0; chan < queue->NumChannels(); chan++) {
                    const void* src = oldqueue->GetRawDequeuePtr(count, chan);
                    void* dst = queue->GetRawEnqueuePtr(count, chan);
                    ASSERT(src && dst);
                    memcpy(dst,src,count);
                }

                queue->Enqueue(count);
                oldqueue->Dequeue(count);
            }

            enqueueUseOld = false;
            delete oldqueue;
            oldqueue = 0;
        } else {
            queue->Enqueue(count);
        }
    }

    unsigned ThresholdQueue::NumChannels() const {
        AutoLock<const QueueBase> al(*this);
        return queue->NumChannels();
    }

    unsigned ThresholdQueue::ChannelStride() const {
        AutoLock<const QueueBase> al(*this);
        return queue->ChannelStride();
    }

    unsigned ThresholdQueue::Freespace() const {
        AutoLock<const QueueBase> al(*this);
        return queue->Freespace();
    }

    bool ThresholdQueue::Full() const {
        AutoLock<const QueueBase> al(*this);
        return queue->Full();
    }

    const void *ThresholdQueue::InternalGetRawDequeuePtr(unsigned thresh, unsigned chan) {
        const void *ret = 0;
        if (dequeueUseOld) {
            // The ONLY reason this code path should be followed is if the node made a getdequeueptr
            // then called getdequeueptr again before dequeue when a grow happens inbetween
            ASSERT(indequeue);
            ret = oldqueue->GetRawDequeuePtr(thresh, chan);
            ASSERT(ret);
        } else {
            ret = queue->GetRawDequeuePtr(thresh, chan);
        }
        return ret;
    }

    void ThresholdQueue::InternalDequeue(unsigned count) {
        if (dequeueUseOld) {
            dequeueUseOld = false;
            delete oldqueue;
            oldqueue = 0;
        }
        queue->Dequeue(count);
    }

    unsigned ThresholdQueue::Count() const {
        AutoLock<const QueueBase> al(*this);
        return queue->Count();
    }

    bool ThresholdQueue::Empty() const {
        AutoLock<const QueueBase> al(*this);
        return queue->Empty();
    }

    unsigned ThresholdQueue::MaxThreshold() const {
        AutoLock<const QueueBase> al(*this);
        return queue->MaxThreshold();
    }

    unsigned ThresholdQueue::QueueLength() const {
        AutoLock<const QueueBase> al(*this);
        return queue->QueueLength();
    }

    unsigned ThresholdQueue::NumEnqueued() const {
        AutoLock<const QueueBase> al(*this);
        return queue->ElementsEnqueued();
    }

    unsigned ThresholdQueue::NumDequeued() const {
        AutoLock<const QueueBase> al(*this);
        return queue->ElementsDequeued();
    }

    void ThresholdQueue::Grow(unsigned queueLen, unsigned maxThresh) {
        AutoLock<QueueBase> al(*this);
        ASSERT(!(inenqueue && indequeue), "Unhandled grow case of having an outstanding dequeue and enqueue");
        if (oldqueue) {
            // If the old queue is still around we have to still be in the same state
            ASSERT(enqueueUseOld == inenqueue);
            ASSERT(dequeueUseOld == indequeue);
            queue->Grow(queueLen, maxThresh, false);
        } else if (!inenqueue && !indequeue) {
            queue->Grow(queueLen, maxThresh, false);
        } else {
            enqueueUseOld = inenqueue;
            dequeueUseOld = indequeue;
            // this should make a duplicate
            oldqueue = queue->Grow(queueLen, maxThresh, true);
            if (!oldqueue) {
                enqueueUseOld = false;
                enqueueUseOld = false;
            }
        }
    }

    ThresholdQueue::TQImpl::TQImpl(unsigned length, unsigned maxthresh, unsigned numchan)
        : ThresholdQueueBase(1, length, maxthresh, numchan)
    {
    }

    ThresholdQueue::TQImpl::TQImpl(const ThresholdQueueAttr &attr)
        : ThresholdQueueBase(1, attr)
    {
    }

    ThresholdQueue::TQImpl *ThresholdQueue::TQImpl::Grow(unsigned queueLen, unsigned maxThresh, bool copy) {
        // ignore the do-nothing case
        if (queueLen <= QueueLength() && maxThresh <= MaxThreshold()) return 0;
        
        // we don't do any shrinking
        if (maxThresh <= MaxThreshold()) maxThresh = MaxThreshold();
        if (queueLen <= QueueLength()) queueLen = QueueLength();
        
        // keep our old info around
        auto_ptr<TQImpl> oldQueue = auto_ptr<TQImpl>(new TQImpl(*this));    // just duplicate the pointers
        // Save the head and tail, these are the only member variables
        // that we care about that will be changed by dequeueing all the data.
        ulong oldhead = head;
        ulong oldtail = tail;
        
        // allocate a new buffer (or MirrorBufferSet)
        AllocateBuf(queueLen,maxThresh,numChannels,mbs?1:0);
        
        // growth should not affect this member
        elementsDequeued = oldQueue->ElementsDequeued();

        // copy all in oldQueue to our new buffer with existing mechanisms
        ulong count;
        while ( (count = oldQueue->Count()) != 0 ) {
            if (count > oldQueue->MaxThreshold()) {
                count = oldQueue->MaxThreshold();
            }
            for (ulong chan = 0; chan < numChannels; chan++) {
                const void* src = oldQueue->GetRawDequeuePtr(count, chan);
                void* dst = GetRawEnqueuePtr(count, chan);
                ASSERT(src && dst);
                memcpy(dst, src, count*elementSize);
            }
            Enqueue(count);
            oldQueue->Dequeue(count);
        }
        
        // growth should not affect this member
        elementsEnqueued = oldQueue->ElementsEnqueued();
        // reset things inside oldQueue to where they where before we copied
        oldQueue->head = oldhead;
        oldQueue->tail = oldtail;

        if (copy) {
            return oldQueue.release();
        }
        return 0;
    }
}

