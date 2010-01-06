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
#include "QueueAttr.h"
#include <cstring>

namespace CPN {

    ThresholdQueue::ThresholdQueue(shared_ptr<Database> db, const SimpleQueueAttr &attr)
        : QueueBase(db, attr),
    // ThresholdQueueBase(ulong elemSize, ulong queueLen, ulong maxThresh, ulong numChans=1);
        queue(1, attr.GetLength(), attr.GetMaxThreshold(), attr.GetNumChannels())
    { }

    ThresholdQueue::~ThresholdQueue() {
    }

    void *ThresholdQueue::InternalGetRawEnqueuePtr(unsigned thresh, unsigned chan) {
        return queue.GetRawEnqueuePtr(thresh, chan);
    }

    void ThresholdQueue::InternalEnqueue(unsigned count) {
        queue.Enqueue(count);
    }

    unsigned ThresholdQueue::NumChannels() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue.NumChannels();
    }

    unsigned ThresholdQueue::ChannelStride() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue.ChannelStride();
    }

    unsigned ThresholdQueue::Freespace() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue.Freespace();
    }

    bool ThresholdQueue::Full() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue.Full();
    }


    // From QueueReader
    const void *ThresholdQueue::InternalGetRawDequeuePtr(unsigned thresh, unsigned chan) {
        return queue.GetRawDequeuePtr(thresh, chan);
    }

    void ThresholdQueue::InternalDequeue(unsigned count) {
        queue.Dequeue(count);
    }

    unsigned ThresholdQueue::Count() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue.Count();
    }

    bool ThresholdQueue::Empty() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue.Empty();
    }

    unsigned ThresholdQueue::MaxThreshold() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue.MaxThreshold();
    }

    unsigned ThresholdQueue::QueueLength() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue.QueueLength();
    }

    unsigned ThresholdQueue::ElementsEnqueued() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue.ElementsEnqueued();
    }

    unsigned ThresholdQueue::ElementsDequeued() const {
        Sync::AutoLock<QueueBase> al(*this);
        return queue.ElementsDequeued();
    }

    void ThresholdQueue::Grow(unsigned queueLen, unsigned maxThresh) {
        Sync::AutoLock<QueueBase> al(*this);
        queue.Grow(queueLen, maxThresh);
    }

}

