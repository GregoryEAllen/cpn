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
 * \brief Implementation of CPN ThresholdQueue
 * \author John Bridgman
 */

#include "ThresholdQueue.h"
#include <cstring>

namespace CPN {

    ThresholdQueue::ThresholdQueue(unsigned size, unsigned maxThresh,
            unsigned numChans) : QueueBase(),
    // ThresholdQueueBase(ulong elemSize, ulong queueLen, ulong maxThresh, ulong numChans=1);
        queue(1, size, maxThresh, numChans)
    { }

    ThresholdQueue::~ThresholdQueue() {
    }

    void* ThresholdQueue::GetRawEnqueuePtr(unsigned thresh, unsigned chan) {
        Sync::AutoReentrantLock l(qlock);
        return queue.GetRawEnqueuePtr(thresh, chan);
    }

    void ThresholdQueue::Enqueue(unsigned count) {
        Sync::AutoReentrantLock l(qlock);
        queue.Enqueue(count);
    }

    bool ThresholdQueue::RawEnqueue(const void* data, unsigned count) {
        return RawEnqueue(data, count, 1, 0);
    }

    bool ThresholdQueue::RawEnqueue(const void* data, unsigned count,
            unsigned numChans, unsigned chanStride) {
        Sync::AutoReentrantLock l(qlock);
        void* dest = queue.GetRawEnqueuePtr(count, 0);
        const void* src = data;
        if (!dest) return false;
        memcpy(dest, src, count);
        for (unsigned i = 1; i < numChans; ++i) {
            dest = queue.GetRawEnqueuePtr(count, i);
            src = ((char*)data) + (chanStride*i);
            memcpy(dest, src, count);
        }
        queue.Enqueue(count);
        return true;
    }

    unsigned ThresholdQueue::NumChannels() const {
        Sync::AutoReentrantLock l(qlock);
        return queue.NumChannels();
    }

    unsigned ThresholdQueue::ChannelStride() const {
        Sync::AutoReentrantLock l(qlock);
        return queue.ChannelStride();
    }

    unsigned ThresholdQueue::Freespace() const {
        Sync::AutoReentrantLock l(qlock);
        return queue.Freespace();
    }

    bool ThresholdQueue::Full() const {
        Sync::AutoReentrantLock l(qlock);
        return queue.Full();
    }


    // From QueueReader
    const void* ThresholdQueue::GetRawDequeuePtr(unsigned thresh, unsigned chan) {
        Sync::AutoReentrantLock l(qlock);
        return queue.GetRawDequeuePtr(thresh, chan);
    }

    void ThresholdQueue::Dequeue(unsigned count) {
        Sync::AutoReentrantLock l(qlock);
        queue.Dequeue(count);
    }

    bool ThresholdQueue::RawDequeue(void* data, unsigned count) {
        return RawDequeue(data, count, 1, 0);
    }

    bool ThresholdQueue::RawDequeue(void* data, unsigned count,
            unsigned numChans, unsigned chanStride) {
        Sync::AutoReentrantLock l(qlock);
        const void* src = queue.GetRawDequeuePtr(count, 0);
        void* dest = data;
        if (!src) return false;
        memcpy(dest, src, count);
        for (unsigned i = 1; i < numChans; ++i) {
            src = queue.GetRawDequeuePtr(count, i);
            dest = ((char*)data) + (chanStride*i);
            memcpy(dest, src, count);
        }
        queue.Dequeue(count);
        return true;
    }

    unsigned ThresholdQueue::Count() const {
        Sync::AutoReentrantLock l(qlock);
        return queue.Count();
    }

    bool ThresholdQueue::Empty() const {
        Sync::AutoReentrantLock l(qlock);
        return queue.Empty();
    }

    unsigned ThresholdQueue::MaxThreshold() const {
        Sync::AutoReentrantLock l(qlock);
        return queue.MaxThreshold();
    }

    unsigned ThresholdQueue::QueueLength() const {
        Sync::AutoReentrantLock l(qlock);
        return queue.QueueLength();
    }

    unsigned ThresholdQueue::ElementsEnqueued() const {
        Sync::AutoReentrantLock l(qlock);
        return queue.ElementsEnqueued();
    }

    unsigned ThresholdQueue::ElementsDequeued() const {
        Sync::AutoReentrantLock l(qlock);
        return queue.ElementsDequeued();
    }

    void ThresholdQueue::Grow(unsigned queueLen, unsigned maxThresh) {
        Sync::AutoReentrantLock l(qlock);
        queue.Grow(queueLen, maxThresh);
    }

}

