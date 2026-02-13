//=============================================================================
//	$Id: ThresholdQueueBase.cc,v 1.1 2006/06/12 20:30:13 gallen Exp $
//-----------------------------------------------------------------------------
//	A queue class that is optimized for DSP applications.
//	Specifically, reading and writing is done directly from queue memory,
//		and data is guaranteed to be contiguous in memory.
//-----------------------------------------------------------------------------
//	The following _must_ hold: 1 <= qMaxThreshold < qLength
//=============================================================================


#include "ThresholdQueue.h"
#include "MirrorBufferSet.h"
#include <stdlib.h>
#include <string.h>


//-----------------------------------------------------------------------------
ThresholdQueueBase::ThresholdQueueBase(ulong elemSize, ulong queueLen,
	ulong maxThresh, ulong numChans)
//-----------------------------------------------------------------------------
:	elementSize(elemSize),
	queueLength(queueLen), 
	maxThreshold(maxThresh),
	numChannels(numChans),
	chanOffset(0), baseOffset(0),
	mbs(0), base(0)
{
	if (maxThreshold<1)
		maxThreshold = 1;
	if (maxThreshold>queueLength)
		queueLength = maxThreshold;

	Reset();
	
	bool useMBS = 1;
	AllocateBuf(queueLength, maxThreshold, numChannels, useMBS);
}

//-----------------------------------------------------------------------------
ThresholdQueueBase::ThresholdQueueBase(ulong elemSize, const ThresholdQueueAttr& attr)
//-----------------------------------------------------------------------------
:	elementSize(elemSize),
	queueLength(attr.queueLength), 
	maxThreshold(attr.maxThreshold),
	numChannels(attr.numChannels),
	chanOffset(0), baseOffset(0),
	mbs(0), base(0)
{
	if (maxThreshold<1)
		maxThreshold = 1;
	if (maxThreshold>queueLength)
		queueLength = maxThreshold;

	Reset();

	bool useMBS = attr.useMBS;
	if (useMBS) {
		// there is no reason for an offset bigger than the page size
		baseOffset = attr.baseOffset % MirrorBufferSet::PageSize();
		baseOffset = baseOffset / elementSize;
		chanOffset = attr.chanOffset  / elementSize;
	}
	
	AllocateBuf(queueLength, maxThreshold, numChannels, useMBS);
}


//-----------------------------------------------------------------------------
void ThresholdQueueBase::AllocateBuf(ulong queueLen, ulong maxThresh,
    ulong numChans, bool useMBS)
//-----------------------------------------------------------------------------
{
    if (maxThresh<1)        maxThresh = 1;
    if (maxThresh>queueLen) queueLen = maxThresh;
    numChannels = numChans;

    if ( useMBS && MirrorBufferSet::Supported() ) {
        ulong bufSz = queueLen * elementSize;
        ulong mirSz = maxThresh-1 + baseOffset + (numChannels-1)*chanOffset;
        mirSz *= elementSize;
        mbs = new MirrorBufferSet(bufSz, mirSz, numChannels);
        // MirrorBufferSet may have just resized everything...
        queueLength  = mbs->BufferSize() / elementSize;
        maxThreshold = mbs->MirrorSize() / elementSize + 1;
        channelStride = queueLength + maxThreshold-1 + chanOffset;
        maxThreshold -= baseOffset + (numChannels-1)*chanOffset;
        base = (char*)((void*)(*mbs)) + baseOffset;
        // Corner case of queueLength == maxThreshold
        if (maxThreshold > queueLength) { maxThreshold = queueLength; }
    } else {
        queueLength = queueLen;
        maxThreshold = maxThresh;
        channelStride = queueLength + maxThreshold - 1;
		base = malloc(channelStride * numChannels * elementSize);
    }
    Reset();
}



//-----------------------------------------------------------------------------
void ThresholdQueueBase::Reset(void)
//-----------------------------------------------------------------------------
{
	elementsEnqueued = 0;
	elementsDequeued = 0;

	head = 0;
	tail = 0;
}


//-----------------------------------------------------------------------------
ThresholdQueueBase::~ThresholdQueueBase(void)
//-----------------------------------------------------------------------------
{
	if (mbs) {
		delete mbs;
		mbs = 0;
	} else {
		free(base);
	}
	base = 0;
}


//-----------------------------------------------------------------------------
void* ThresholdQueueBase::GetRawEnqueuePtr(ulong thresh, ulong chan) const
//	get a pointer for writing data before calling Enqueue
//-----------------------------------------------------------------------------
{
	if (thresh>Freespace() || thresh>MaxThreshold()) return 0;
//	if (chan>=numChannels) return 0;
//	assert(chan<numChannels);
	register ulong idx = tail;
	while (idx>=queueLength) idx -= queueLength;
	return (char*) base + (chan*channelStride + idx) * elementSize;
}


//-----------------------------------------------------------------------------
const void* ThresholdQueueBase::GetRawDequeuePtr(ulong thresh, ulong chan) const
//	get the current pointer (to thresh valid samples)
//-----------------------------------------------------------------------------
{
	if (thresh>Count() || thresh>MaxThreshold()) return 0;
//	if (chan>=numChannels) return 0;
//	assert(chan<numChannels);	
	register ulong idx = head;
	while (idx>=queueLength) idx -= queueLength;
	return (char*) base + (chan*channelStride + idx) * elementSize;
}


//-----------------------------------------------------------------------------
void ThresholdQueueBase::Enqueue(ulong count)
//	move all of the data to the right place, then update the indices
//-----------------------------------------------------------------------------
{
	if (!mbs) {	// the mbs maintains circularity
	
		// we must mirror by ourselves
		register ulong idx = tail;
		while (idx>=queueLength) idx -= queueLength;

		// elems to copy from queue area to mirror area
		register ulong countUp = 0;
		if (idx<maxThreshold-1) countUp = maxThreshold-1-idx;	// to mirror's lower edge
		if (idx+count<maxThreshold-1) countUp = count;

		// elems to copy from mirror area to queue area
		register ulong countDn = 0;
		if (idx+count>queueLength) countDn = idx+count-queueLength;
		
		if (countUp || countDn)
			for (ulong chan=0; chan<numChannels; chan++) {
				char* chanBase = (char*)base + (chan*channelStride) * elementSize;

				// move data in the queue (below the threshold) to the mirror area
				char* dst = chanBase + (idx + queueLength) * elementSize;
				char* src = chanBase + idx * elementSize;
				memcpy(dst,src,countUp*elementSize);
				
				// move data from the mirror area to the queue (below the threshold)
				src = chanBase + queueLength * elementSize;
				memcpy(chanBase,src,countDn*elementSize);
			}
	}
	
	// update the tail pointer
	register ulong newTail = tail+count;
	while (newTail>=2*queueLength) newTail -= 2*queueLength;
	tail = newTail;
	elementsEnqueued += count;
}


//-----------------------------------------------------------------------------
void ThresholdQueueBase::Dequeue(ulong count)
//	release the buffer just referenced by GetDequeuePtr
//-----------------------------------------------------------------------------
{
	register ulong newHead = head+count;
	while (newHead>=2*queueLength) newHead -= 2*queueLength;
	head = newHead;
	elementsDequeued += count;
}


//-----------------------------------------------------------------------------
ThresholdQueueBase::ulong ThresholdQueueBase::Count(void) const
//	the number of elements in the queue
//-----------------------------------------------------------------------------
{
	register ulong count = (2*queueLength - head + tail);
	while (count>=queueLength) count -= queueLength;
	if (count) return count;
	return (head==tail) ? 0 : queueLength;
}


//-----------------------------------------------------------------------------
ThresholdQueueBase::ulong ThresholdQueueBase::Freespace(void) const
//	the number of elements free in the queue
//-----------------------------------------------------------------------------
{
	return queueLength - Count();
}


#if 1
#include <assert.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
void ThresholdQueueBase::Grow(ulong queueLen, ulong maxThresh)
//	Growing is only supposed to occur when a system is:
//		"deadlocked, trying to write to a full queue"
//	This is, by definition, an expensive operation. Little effort has been
//		spent optimizing for this case
//-----------------------------------------------------------------------------
{
	fprintf(stderr, "ThresholdQueueBase::Grow(%lu,%lu)\n", queueLen, maxThresh);

	// handle the default parameter case
	if (!maxThresh) maxThresh = MaxThreshold();
	
	// ignore the do-nothing case
	if (queueLen <= QueueLength() && maxThresh <= MaxThreshold()) return;
	
	// we don't do any shrinking
	if (maxThresh <= MaxThreshold()) maxThresh = MaxThreshold();
	if (queueLen <= QueueLength()) queueLen = QueueLength();
	
	// keep our old info around
	ThresholdQueueBase	oldQueue(*this);	// just duplicate the pointers
	
	// allocate a new buffer (or MirrorBufferSet)
	AllocateBuf(queueLen,maxThresh,numChannels,mbs?1:0);
	
	// growth should not affect this member
	elementsDequeued = oldQueue.ElementsDequeued();

	// copy all in oldQueue to our new buffer with existing mechanisms
	ulong count;
	while (count = oldQueue.Count()) {
		if (count>oldQueue.MaxThreshold())
			count = oldQueue.MaxThreshold();
		for (ulong chan=0; chan<numChannels; chan++) {
			const void* src = oldQueue.GetRawDequeuePtr(count,chan);
			void* dst = GetRawEnqueuePtr(count,chan);
			assert(src && dst);
			memcpy(dst,src,count*elementSize);
		}
		Enqueue(count);
		oldQueue.Dequeue(count);
	}
	
	// growth should not affect this member
	elementsEnqueued = oldQueue.ElementsEnqueued();
	
	// when oldQueue gets destructed, it will deallocate the old buffers
}
#endif
