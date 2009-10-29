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
 * \brief A simple implementation of the QueueBase interface.
 * \author John Bridgman
 */

#ifndef CPN_SIMPLEQUEUE_H
#define CPN_SIMPLEQUEUE_H
#pragma once

#include "CPNCommon.h"
#include "QueueBase.h"
#include "ReentrantLock.h"
#include "CircularQueue.h"

namespace CPN {

    /**
     * /brief A very simple implementatin of QueueBase.
     * 
     * Based on the ::SimpleQueue queue.
     *
     * \see QueueBase
     */
    class CPN_LOCAL SimpleQueue : public QueueBase {
    public:

        SimpleQueue(unsigned size, unsigned maxThresh, unsigned numChans)
            : queue(size, maxThresh, numChans) {}
        ~SimpleQueue();

        void* GetRawEnqueuePtr(unsigned thresh, unsigned chan=0) {
            Sync::AutoReentrantLock l(lock);
            return queue.GetRawEnqueuePtr(thresh, chan);
        }

        void Enqueue(unsigned count) {
            Sync::AutoReentrantLock l(lock);
            queue.Enqueue(count);
        }

        bool RawEnqueue(const void* data, unsigned count) {
            Sync::AutoReentrantLock l(lock);
            return queue.RawEnqueue(data, count);
        }
        bool RawEnqueue(const void* data, unsigned count, unsigned numChans, unsigned chanStride) {
            Sync::AutoReentrantLock l(lock);
            return queue.RawEnqueue(data, count, numChans, chanStride);
        }
        const void* GetRawDequeuePtr(unsigned thresh, unsigned chan=0) {
            Sync::AutoReentrantLock l(lock);
            return queue.GetRawDequeuePtr(thresh, chan);
        }
        void Dequeue(unsigned count) {
            Sync::AutoReentrantLock l(lock);
            queue.Dequeue(count);
        }
        bool RawDequeue(void* data, unsigned count) {
            Sync::AutoReentrantLock l(lock);
            return queue.RawDequeue(data, count);
        }
        bool RawDequeue(void* data, unsigned count, unsigned numChans, unsigned chanStride) {
            Sync::AutoReentrantLock l(lock);
            return RawDequeue(data, count, numChans, chanStride);
        }

        unsigned NumChannels() const {
            Sync::AutoReentrantLock l(lock);
            return queue.NumChannels();
        }
        unsigned MaxThreshold() const {
            Sync::AutoReentrantLock l(lock);
            return queue.MaxThreshold();
        }
        unsigned QueueLength() const {
            Sync::AutoReentrantLock l(lock);
            return queue.QueueLength();
        }
        unsigned Freespace() const {
            Sync::AutoReentrantLock l(lock);
            return queue.Freespace();
        }
        bool Full() const {
            Sync::AutoReentrantLock l(lock);
            return queue.Full();
        }
        unsigned Count() const {
            Sync::AutoReentrantLock l(lock);
            return queue.Count();
        }
        bool Empty() const {
            Sync::AutoReentrantLock l(lock);
            return queue.Empty();
        }

        unsigned ChannelStride() const {
            Sync::AutoReentrantLock l(lock);
            return queue.ChannelStride();
        }

        void Grow(unsigned queueLen, unsigned maxThresh) {
            Sync::AutoReentrantLock l(lock);
            queue.Grow(queueLen, maxThresh);
        }

    private:
        ::CircularQueue queue;
    };
}

#endif

