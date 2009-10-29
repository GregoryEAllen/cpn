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
 * \brief A simple circular queue similar to ThresholdQueue.
 * \author John Bridgman
 */

#ifndef CIRCULARQUEUE_H
#define CIRCULARQUEUE_H
#pragma once

#include "AutoBuffer.h"

/**
 * /brief A very simple circular queue.
 * 
 * No special memory mapping or any other stuff, just a
 * plain memory buffer. Multiple calls the Enqueue are not
 * checked for invalid input. So the user can try to 
 * submit more than there queue space and predictable
 * errors will follow.
 *
 * Unlike ThresholdQueue this does not do any memory mapping so
 * has no minimum size.
 *
 */
class CircularQueue {
public:

    CircularQueue(unsigned size, unsigned maxThresh, unsigned numChans);
    ~CircularQueue();

    void* GetRawEnqueuePtr(unsigned thresh, unsigned chan=0);
    void Enqueue(unsigned count);
    bool RawEnqueue(const void* data, unsigned count);
    bool RawEnqueue(const void* data, unsigned count, unsigned numChans, unsigned chanStride);
    const void* GetRawDequeuePtr(unsigned thresh, unsigned chan=0);
    void Dequeue(unsigned count);
    bool RawDequeue(void* data, unsigned count);
    bool RawDequeue(void* data, unsigned count, unsigned numChans, unsigned chanStride);

    unsigned NumChannels() const;
    unsigned MaxThreshold() const;
    unsigned QueueLength() const;
    unsigned Freespace() const;
    bool Full() const;
    unsigned Count() const;
    bool Empty() const;

    unsigned ChannelStride() const;

    // Only increases size
    void Grow(unsigned queueLen, unsigned maxThresh);

private:
    unsigned queueLength;
    unsigned maxThreshold;
    const unsigned numChannels;
    unsigned chanStride;
    // head points to the next empty byte
    unsigned head;
    // tail points to the next byte to read
    unsigned tail;
    AutoBuffer buffer;
};

#endif

