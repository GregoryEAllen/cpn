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

namespace CPN {

    SimpleQueue::SimpleQueue(unsigned size, unsigned maxThresh, unsigned numChans)
        : queue(size, maxThresh, numChans) {}

    SimpleQueue::~SimpleQueue() {
    }

    void* SimpleQueue::GetRawEnqueuePtr(unsigned thresh, unsigned chan) {
        Sync::AutoReentrantLock l(lock);
        return queue.GetRawEnqueuePtr(thresh, chan);
    }

    void SimpleQueue::Enqueue(unsigned count) {
        Sync::AutoReentrantLock l(lock);
        queue.Enqueue(count);
    }

    bool SimpleQueue::RawEnqueue(const void* data, unsigned count) {
        Sync::AutoReentrantLock l(lock);
        return queue.RawEnqueue(data, count);
    }

    bool SimpleQueue::RawEnqueue(const void* data, unsigned count, unsigned numChans, unsigned chanStride) {
        Sync::AutoReentrantLock l(lock);
        return queue.RawEnqueue(data, count, numChans, chanStride);
    }
    const void* SimpleQueue::GetRawDequeuePtr(unsigned thresh, unsigned chan) {
        Sync::AutoReentrantLock l(lock);
        return queue.GetRawDequeuePtr(thresh, chan);
    }
    void SimpleQueue::Dequeue(unsigned count) {
        Sync::AutoReentrantLock l(lock);
        queue.Dequeue(count);
    }
    bool SimpleQueue::RawDequeue(void* data, unsigned count) {
        Sync::AutoReentrantLock l(lock);
        return queue.RawDequeue(data, count);
    }
    bool SimpleQueue::RawDequeue(void* data, unsigned count, unsigned numChans, unsigned chanStride) {
        Sync::AutoReentrantLock l(lock);
        return queue.RawDequeue(data, count, numChans, chanStride);
    }

    unsigned SimpleQueue::NumChannels() const {
        Sync::AutoReentrantLock l(lock);
        return queue.NumChannels();
    }

    unsigned SimpleQueue::MaxThreshold() const {
        Sync::AutoReentrantLock l(lock);
        return queue.MaxThreshold();
    }

    unsigned SimpleQueue::QueueLength() const {
        Sync::AutoReentrantLock l(lock);
        return queue.QueueLength();
    }

    unsigned SimpleQueue::Freespace() const {
        Sync::AutoReentrantLock l(lock);
        return queue.Freespace();
    }

    bool SimpleQueue::Full() const {
        Sync::AutoReentrantLock l(lock);
        return queue.Full();
    }

    unsigned SimpleQueue::Count() const {
        Sync::AutoReentrantLock l(lock);
        return queue.Count();
    }

    bool SimpleQueue::Empty() const {
        Sync::AutoReentrantLock l(lock);
        return queue.Empty();
    }

    unsigned SimpleQueue::ChannelStride() const {
        Sync::AutoReentrantLock l(lock);
        return queue.ChannelStride();
    }

    void SimpleQueue::Grow(unsigned queueLen, unsigned maxThresh) {
        Sync::AutoReentrantLock l(lock);
        queue.Grow(queueLen, maxThresh);
    }

}

