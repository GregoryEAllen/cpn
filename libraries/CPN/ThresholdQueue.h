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
 * \brief Default threshold queue implementation.
 * \author John Bridgman
 */

#ifndef CPN_THRESHOLDQUEUE_H
#define CPN_THRESHOLDQUEUE_H
#pragma once

#include "CPNCommon.h"
#include "ThresholdQueueBase.h"
#include "QueueBase.h"

namespace CPN {

    /**
     * \brief A version of the ThresholdQueue that provides the
     * CPN Queue interface
     * This queue implementation creates a memory mapped object
     * which has as a minimum size the machine page size. So there is
     * an effective minimum queue size. Any queue size less than
     * the page size will be expanded to the page size.
     *
     * \see QueueBase and ThresholdQueueBase
     */
    class CPN_LOCAL ThresholdQueue : public QueueBase {
    public:
        ThresholdQueue(KernelBase *k, const SimpleQueueAttr &attr);
        ThresholdQueue(KernelBase *k, const SimpleQueueAttr &attr, unsigned length);
        ~ThresholdQueue();

    protected:
        virtual void *InternalGetRawEnqueuePtr(unsigned thresh, unsigned chan);
        virtual void InternalEnqueue(unsigned count);


        virtual const void *InternalGetRawDequeuePtr(unsigned thresh, unsigned chan);
        virtual void InternalDequeue(unsigned count);

        virtual unsigned UnlockedNumChannels() const;
        virtual unsigned UnlockedMaxThreshold() const;
        virtual unsigned UnlockedQueueLength() const;
        virtual unsigned UnlockedFreespace() const;
        virtual bool UnlockedFull() const;
        virtual unsigned UnlockedCount() const;
        virtual bool UnlockedEmpty() const;
        virtual unsigned UnlockedChannelStride() const;

        unsigned UnlockedNumEnqueued() const;
        unsigned UnlockedNumDequeued() const;

        virtual void UnlockedGrow(unsigned queueLen, unsigned maxThresh);

    protected:
        /**
         * The actual queue implementation.
         */
        class TQImpl : public ThresholdQueueBase {
        public:
            typedef ThresholdQueueBase::ulong ulong;
            TQImpl(unsigned length, unsigned maxthres, unsigned numchan);
            TQImpl(const ThresholdQueueAttr &attr);

            TQImpl *Grow(unsigned queueLen, unsigned maxThresh, bool copy);
        };
        TQImpl *queue;
        TQImpl *oldqueue;
        bool enqueueUseOld;
        bool dequeueUseOld;
    };

}
#endif
