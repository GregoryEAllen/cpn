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
    head(0), tail(0), buffer((size + 1 + maxThresh)*numChans)
{
    if (size < maxThresh) {
        queueLength = maxThresh + 1;
        chanStride = 2 * maxThresh + 1;
        buffer.resize(chanStride * numChannels);
    }
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
        return &buffer[head + (chanStride)*chan];
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
            memcpy(&buffer[chanOff],
                   &buffer[queueLength + chanOff],
                   newHead);
        }
    }
    head = newHead;
}

bool CircularQueue::RawEnqueue(const void* data, unsigned count,
        unsigned numChans, unsigned chanStride) {
    ASSERT(numChans <= numChannels);
    if (Freespace() < count) { return false; }
    unsigned numcopied = 0;
    while (numcopied < count) {
        unsigned numtocopy = count - numcopied;
        if (numtocopy > MaxThreshold()) { numtocopy = MaxThreshold(); }
        char *dest = (char*)GetRawEnqueuePtr(numtocopy, 0);
        const char* src = ((const char*)data) + numcopied;
        ASSERT(dest);
        memcpy(dest, src, numtocopy);
        for (unsigned chan = 1; chan < numChans; ++chan) {
            dest = (char*)GetRawEnqueuePtr(numtocopy, chan);
            src += chanStride;
            memcpy(dest, src, numtocopy);
        }
        Enqueue(numtocopy);
        numcopied += numtocopy;
    }
    return true;
}

const void* CircularQueue::GetRawDequeuePtr(unsigned thresh, unsigned chan) {
    unsigned long chanOff = (chanStride)*chan;
    if (Count() >= thresh && maxThreshold >= thresh) {
        if (tail + thresh > queueLength) {
            memcpy(&buffer[queueLength + chanOff],
                   &buffer[chanOff],
                   tail + thresh - queueLength);
        }
        return &buffer[tail + chanOff];
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

bool CircularQueue::RawDequeue(void *data, unsigned count, unsigned numChans, unsigned chanStride) {
    ASSERT(numChans <= numChannels);
    if (Count() < count) { return false; }
    unsigned numcopied = 0;
    while (numcopied < count) {
        unsigned numtocopy = count - numcopied;
        if (numtocopy > MaxThreshold()) { numtocopy = MaxThreshold(); }
        const char *src = (const char*)GetRawDequeuePtr(numtocopy, 0);
        char *dest = ((char*)data) + numcopied;
        ASSERT(src);
        memcpy(dest, src, numtocopy);
        for (unsigned chan = 1; chan < numChans; ++chan) {
            src = (const char*)GetRawDequeuePtr(numtocopy, chan);
            dest += chanStride;
            memcpy(dest, src, numtocopy);
        }
        Dequeue(numtocopy);
        numcopied += numtocopy;
    }
    return true;
}

unsigned CircularQueue::Freespace() const {
    if (head >= tail) {
        return queueLength - (head - tail) - 1;
    } else {
        return tail - head - 1;
    }
}

unsigned CircularQueue::Count() const {
    if (head >= tail) {
        return head - tail;
    } else {
        return head + (queueLength - tail);
    }
}

void CircularQueue::Grow(unsigned queueLen, unsigned maxThresh) {
    // Enforce interface contract of not reducing size
    if (queueLen <= queueLength && maxThresh <= maxThreshold) return;
    if (queueLen < queueLength) { queueLen = queueLength; }
    if (maxThresh < maxThreshold) { maxThresh = maxThreshold; }
    unsigned newchanstride = queueLen + 1 + maxThresh;
    std::vector<char> copybuffer(newchanstride * NumChannels());
    unsigned oldcount = Count();
    ENSURE(RawDequeue(&copybuffer[0], oldcount, NumChannels(), newchanstride));
    tail = 0;
    queueLength = queueLen + 1;
    maxThreshold = maxThresh;
    chanStride = newchanstride;
    head = oldcount;
    buffer.swap(copybuffer);
}

