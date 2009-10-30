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

#include "CircularQueue.h"
#include "Assert.h"
#include <cstring>


CircularQueue::CircularQueue(unsigned size, unsigned maxThresh, unsigned numChans)
       : queueLength(size + 1),
    maxThreshold(maxThresh), numChannels(numChans),
    chanStride(size + 1 + maxThresh),
    head(0), tail(0), buffer((size + 1 + maxThresh)*numChans) {
}

CircularQueue::~CircularQueue() {
    maxThreshold = 0;
    chanStride = 0;
    head = 0;
    tail = 0;
}

void* CircularQueue::GetRawEnqueuePtr(unsigned thresh, unsigned chan) {
    ASSERT(chan < numChannels);
    if (Freespace() >= thresh && maxThreshold >= thresh) {
        return buffer.GetBuffer(head + (chanStride)*chan);
    } else {
        return 0;
    }
}

void CircularQueue::Enqueue(unsigned count) {
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

bool CircularQueue::RawEnqueue(const void* data, unsigned count) {
    return RawEnqueue(data, count, 1, 0);
}

bool CircularQueue::RawEnqueue(const void* data, unsigned count,
        unsigned numChans, unsigned chanStride) {
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


unsigned CircularQueue::NumChannels() const {
    return numChannels;
}

unsigned CircularQueue::Freespace() const {
    if (head >= tail) {
        return queueLength - (head - tail) - 1;
    } else {
        return tail - head - 1;
    }
}

bool CircularQueue::Full() const {
    return Freespace() == 0;
}

const void* CircularQueue::GetRawDequeuePtr(unsigned thresh, unsigned chan) {
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

void CircularQueue::Dequeue(unsigned count) {
    ASSERT(count <= Count());
    unsigned long newTail = tail + count;
    if (newTail >= queueLength) {
        newTail -= queueLength;
    }
    tail = newTail;
}

bool CircularQueue::RawDequeue(void* data, unsigned count) {
    return RawDequeue(data, count, 1, 0);
}

bool CircularQueue::RawDequeue(void* data, unsigned count,
        unsigned numChans, unsigned chanStride) {
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

unsigned CircularQueue::Count() const {
    if (head >= tail) {
        return head - tail;
    } else {
        return head + (queueLength - tail);
    }
}

bool CircularQueue::Empty() const {
    return head == tail;
}

unsigned CircularQueue::ChannelStride() const {
    return chanStride;
}

unsigned CircularQueue::MaxThreshold() const {
    return maxThreshold;
}

unsigned CircularQueue::QueueLength() const {
    return queueLength - 1;
}

void CircularQueue::Grow(unsigned queueLen, unsigned maxThresh) {
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

void CircularQueue::Clear() {
    head = tail = 0;
}

