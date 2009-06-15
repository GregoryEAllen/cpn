//=============================================================================
//	$Id: ThresholdQueueBase.h,v 1.1 2006/06/12 20:30:13 gallen Exp $
//-----------------------------------------------------------------------------
//	A queue class that is optimized for DSP applications.
//	Specifically, reading and writing is done directly from queue memory,
//		and data is guaranteed to be contiguous in memory.
//-----------------------------------------------------------------------------
//	The following _must_ hold: 1 <= qMaxThreshold < qLength
//=============================================================================


#ifndef ThresholdQueueBase_h
#define ThresholdQueueBase_h

//-----------------------------------------------------------------------------
//	Implementation notes:
//
//	+-------------------------+--------------+------
//	|       queueLength       | mirrorLength | ...
//	+-------------------------+--------------+------
//
//	This works like a normal queue, but is intended to make up for the lack
//		of circular address buffers on a general purpose processor.
//	Where supported, MirrorBufferSet will be used, providing the mirroring in
//		hardware with the virtual memory manager.
//	Without MirrorBufferSet, there is a memory vs. performance tradeoff.
//		When queueLength>>mirrorLength, data copying is seldom necessary.
//		When queueLength==mirrorLength, copying must happen very frequently.
//-----------------------------------------------------------------------------
//	This class follows the Computational Process Network calling conventions:
//	- GetDequeuePtr(<thresh>) returns a pointer to <thresh> contiguous elements
//		which are the next elements to be dequeued. That is, <thresh> elements
//		must be in the queue before the consumer can fire. This threshold is
//		equivalent to the [CG] parameter 'T'.
//	- Dequeue(<count>) releases <count> elements from the queue. This <count>
//		is equivalent to the [CG] parameter 'W'. Clearly, T >= W.
//	- GetEnqueuePtr(<thresh>) returns a pointer to space in the queue for
//		<thresh> contiguous elements, which are the next elements to be
//		enqueued. That is, space for <thresh> elements must be in the queue
//		before the producer can fire. (There is no equivalent in [CG].)
//	- Enqueue(<count>) inserts <count> elements into the queue. This <count>
//		is equivalent to the [CG] parameter 'U'. Clearly, enqueueThresh >= U.
//-----------------------------------------------------------------------------
//	[CG] is Karp and Miller's Computation Graphs
//-----------------------------------------------------------------------------
//	queueLength		- The max number of elements in the queue
//	dequeueThresh	- The number of elements guaranteed contiguous beyond
//						a pointer returned by GetDequeuePtr()
//	enqueueThresh	- The number of elements which may be written contiguously
//						beyond a pointer returned by GetEnqueuePtr()
//	maxThreshold	- The max allowable value for dequeueThresh or
//						enqueueThresh. Also, mirrorLength = maxThreshold-1
//-----------------------------------------------------------------------------
//	To dequeue data:
//		int count = <dequeueChunkSize>;
//		T* ptr = theQueue.GetDequeuePtr(count);
//		if (ptr) {
//			for (int i=0; i<count; i++)
//				<outgoingData> = ptr[i];
//			Dequeue(count);
//		}
//	To enqueue data:
//		int count = <enqueueChunkSize>;
//		T* ptr = theQueue.GetEnqueuePtr(count);
//		if (ptr) {
//			for (int i=0; i<count; i++)
//				ptr[i] = <incomingData>;
//			Enqueue(count);
//		}
//-----------------------------------------------------------------------------
// Other constructor parameters:
//	useMBS		- use MirrorBufferSet to get mirroring with virtual memory
//	chanOffset	- additional channel-to-channel offset to reduce cache thrashing
//	baseOffset	- additional base offset to reduce cache thrashing
//		These offsets are in bytes, and do not apply unless useVMM=1
//		These are performance tuning parameters. They should be a multiple of
//			the number of bytes in a cache line, and a multiple of sizeof(T)
//-----------------------------------------------------------------------------

#include "ThresholdQueueAttr.h"

class MirrorBufferSet;

class ThresholdQueueBase {
	// C++ templates are compile-time static, so we must be able to
	// do everything in ThresholdQueueBase
  public:
	typedef unsigned long ulong;
	
	ThresholdQueueBase(ulong elemSize, ulong queueLen, ulong maxThresh, ulong numChans=1);
	ThresholdQueueBase(ulong elemSize, const ThresholdQueueAttr& attr);
	~ThresholdQueueBase(void);

	void	Reset(void);

	void*	GetRawEnqueuePtr(ulong enqueueThresh, ulong chan=0) const;
	void	Enqueue(ulong count);		// insert count after writing to enqueue ptr
	
	const void* GetRawDequeuePtr(ulong dequeueThresh, ulong chan=0) const;
	void	Dequeue(ulong count);		// release count after reading from dequeue ptr

	ulong	Count(void) const;			// elements in the queue
	ulong	Freespace(void) const;		// elements the queue can still hold

	bool	Empty(void) const	{ return !Count(); }
	bool	Full(void) const	{ return !Freespace(); }

	ulong	QueueLength(void) const		{ return queueLength; }
	ulong	MaxThreshold(void) const	{ return maxThreshold; }

	ulong	NumChannels(void) const		{ return numChannels; }
	ulong	ChannelStride(void) const	{ return channelStride; }

	// just for fun
	ulong	ElementsEnqueued(void) const 	{ return elementsEnqueued; }
	ulong	ElementsDequeued(void) const 	{ return elementsDequeued; }

	void	Grow(ulong queueLen, ulong maxThresh);

  protected:
  	ulong	elementSize;	// size of a single element
	ulong	head, tail;		// dequeue & enqueue index
	ulong	queueLength, maxThreshold;
	ulong	numChannels, channelStride;
	ulong	chanOffset, baseOffset;
	ulong	elementsEnqueued, elementsDequeued;
	void*	base;
	MirrorBufferSet*	mbs;
	
	void AllocateBuf(ulong queueLen, ulong maxThresh, ulong numChans, bool useMBS);
};


#endif
