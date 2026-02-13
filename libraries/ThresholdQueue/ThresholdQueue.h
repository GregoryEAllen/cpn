//=============================================================================
//	$Id: ThresholdQueue.h,v 1.1 2006/06/12 20:30:13 gallen Exp $
//-----------------------------------------------------------------------------
//	A queue class that is optimized for DSP applications.
//	Specifically, reading and writing is done directly from queue memory,
//		and data is guaranteed to be contiguous in memory.
//-----------------------------------------------------------------------------
//	The following _must_ hold: 1 <= qMaxThreshold < qLength
//=============================================================================


#ifndef ThresholdQueue_h
#define ThresholdQueue_h

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
//	useVMM		- use MirrorBufferSet to get mirroring with virtual memory
//	chanOffset	- additional channel-to-channel offset to reduce cache thrashing
//	baseOffset	- additional base offset to reduce cache thrashing
//		These offsets are in bytes, and do not apply unless useVMM=1
//		These are performance tuning parameters. They should be a multiple of
//			the number of bytes in a cache line, and a multiple of sizeof(T)
//-----------------------------------------------------------------------------

#include "ThresholdQueueBase.h"


template<class T>
class ThresholdQueue : public ThresholdQueueBase {
  public:
	ThresholdQueue(ulong queueLen, ulong maxThresh, ulong numChans=1)
		: ThresholdQueueBase(sizeof(T), queueLen, maxThresh, numChans) { }
	ThresholdQueue(const ThresholdQueueAttr& attr)
		: ThresholdQueueBase(sizeof(T), attr) { }

	T* GetEnqueuePtr(ulong enqueueThresh, ulong chan=0) const
		{ return (T*) GetRawEnqueuePtr(enqueueThresh,chan); }

	const T* GetDequeuePtr(ulong dequeueThresh, ulong chan=0) const
		{ return (const T*) GetRawDequeuePtr(dequeueThresh,chan); }
};

#endif
