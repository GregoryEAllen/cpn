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
 * \brief Implementation for the simple queue.
 *
 * This implementation adds 1 byte to the size to use as a sentinel
 * to indicate the queue is full.
 *
 * \author John Bridgman
 */

#include "SimpleQueue.h"
#include "Assert.h"
#include <cstring>

namespace CPN {

    SimpleQueue::SimpleQueue(unsigned size, unsigned maxThresh, unsigned numChans)
           : QueueBase(), queueLength(size + 1),
        maxThreshold(maxThresh), numChannels(numChans),
        chanStride(size + 1 + maxThresh),
        head(0), tail(0), buffer((size + 1 + maxThresh)*numChans) {
    }

    SimpleQueue::~SimpleQueue() {
        Sync::AutoReentrantLock l(lock);
        maxThreshold = 0;
        chanStride = 0;
        head = 0;
        tail = 0;
    }

    void* SimpleQueue::GetRawEnqueuePtr(unsigned thresh, unsigned chan) {
        Sync::AutoReentrantLock l(lock);
        ASSERT(chan < numChannels);
        if (Freespace() >= thresh && maxThreshold >= thresh) {
            return buffer.GetBuffer(head + (chanStride)*chan);
        } else {
            return 0;
        }
    }

    void SimpleQueue::Enqueue(unsigned count) {
        Sync::AutoReentrantLock l(lock);
        ASSERT(count <= Freespace());
        unsigned newHead = head + count;
        if (newHead >= queueLength) {
            newHead -= queueLength;
            for (unsigned chan = 0; chan < numChannels; ++chan) {
                unsigned chanOff = (chanStride)*chan;
                memcpy(buffer.GetBuffer(chanOff),
                       buffer.GetBuffer(queueLength + chanOff),
                       newHead);
            }
        }
        head = newHead;
    }

    bool SimpleQueue::RawEnqueue(const void* data, unsigned count) {
        return RawEnqueue(data, count, 1, 0);
    }

    bool SimpleQueue::RawEnqueue(const void* data, unsigned count,
            unsigned numChans, unsigned chanStride) {
        Sync::AutoReentrantLock l(lock);
        ASSERT(numChans <= numChannels);
        void* dest = GetRawEnqueuePtr(count, 0);
        const void* src = data;
        if (!dest) { return false; }
        memcpy(dest, src, count);
        for (unsigned chan = 1; chan < numChans; ++chan) {
            dest = GetRawEnqueuePtr(count, chan);
            src = ((char*)data) + chanStride*chan;
            memcpy(dest, src, count);
        }
        Enqueue(count);
        return true;
    }


    unsigned SimpleQueue::NumChannels() const {
        Sync::AutoReentrantLock l(lock);
        return numChannels;
    }

    unsigned SimpleQueue::Freespace() const {
        Sync::AutoReentrantLock l(lock);
        if (head >= tail) {
            return queueLength - (head - tail) - 1;
        } else {
            return tail - head - 1;
        }
    }

    bool SimpleQueue::Full() const {
        return Freespace() == 0;
    }

    const void* SimpleQueue::GetRawDequeuePtr(unsigned thresh, unsigned chan) {
        Sync::AutoReentrantLock l(lock);
        unsigned long chanOff = (chanStride)*chan;
        if (Count() >= thresh && maxThreshold >= thresh) {
            if (tail + thresh > queueLength) {
                memcpy(buffer.GetBuffer(queueLength + chanOff),
                       buffer.GetBuffer(chanOff),
                       tail + thresh - queueLength);
            }
            return buffer.GetBuffer(tail + chanOff);
        } else {
            return 0;
        }
    }

    void SimpleQueue::Dequeue(unsigned count) {
        Sync::AutoReentrantLock l(lock);
        ASSERT(count <= Count());
        unsigned long newTail = tail + count;
        if (newTail >= queueLength) {
            newTail -= queueLength;
        }
        tail = newTail;
    }

    bool SimpleQueue::RawDequeue(void* data, unsigned count) {
        return RawDequeue(data, count, 1, 0);
    }

    bool SimpleQueue::RawDequeue(void* data, unsigned count,
            unsigned numChans, unsigned chanStride) {
        Sync::AutoReentrantLock l(lock);
        ASSERT(numChans <= numChannels);
        const void* src = GetRawDequeuePtr(count, 0);
        void* dest = data;
        if (!src) { return false; }
        memcpy(dest, src, count);
        for (unsigned chan = 1; chan < numChans; ++chan) {
            src = GetRawDequeuePtr(count, chan);
            dest = ((char*)data) + chanStride*chan;
            memcpy(dest, src, count);
        }
        Dequeue(count);
        return true;
    }

    unsigned SimpleQueue::Count() const {
        Sync::AutoReentrantLock l(lock);
        if (head >= tail) {
            return head - tail;
        } else {
            return head + (queueLength - tail);
        }
    }

    bool SimpleQueue::Empty() const {
        Sync::AutoReentrantLock l(lock);
        return head == tail;
    }

    unsigned SimpleQueue::ChannelStride() const {
        Sync::AutoReentrantLock l(lock);
        return chanStride;
    }

    unsigned SimpleQueue::MaxThreshold() const {
        Sync::AutoReentrantLock l(lock);
        return maxThreshold;
    }

    unsigned SimpleQueue::QueueLength() const {
        Sync::AutoReentrantLock l(lock);
        return queueLength - 1;
    }

    void SimpleQueue::Grow(unsigned queueLen, unsigned maxThresh) {
        Sync::AutoReentrantLock l(lock);
        // Enforce interface contract of not reducing size
        if (queueLen < queueLength && maxThresh < maxThreshold) return;
        if (queueLen < queueLength) { queueLen = queueLength; }
        if (maxThresh < maxThreshold) { maxThresh = maxThreshold; }
        AutoBuffer copybuff = AutoBuffer(buffer.GetSize());
        unsigned oldcount = Count();
        unsigned oldchanstride = ChannelStride();
        RawDequeue(copybuff.GetBuffer(), oldcount, NumChannels(), oldchanstride);
        head = tail = 0;
        queueLength = queueLen;
        maxThreshold = maxThresh;
        chanStride = queueLength + maxThreshold;
        RawEnqueue(copybuff.GetBuffer(), oldcount, NumChannels(), oldchanstride);
    }
}

