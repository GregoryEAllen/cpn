/** \file
 * \brief Default threshold queue implementation.
 */

#ifndef CPN_QUEUE_THRESHOLDQUEUE_H
#define CPN_QUEUE_THRESHOLDQUEUE_H

#include "ThresholdQueueBase.h"
#include "QueueBase.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "common.h"

namespace CPN {

	/**
	 * A version of the ThresholdQueue that provides the
	 * CPN Queue interface
	 */
	class ThresholdQueue : public QueueBase {
	public:
		ThresholdQueue(const QueueAttr &attr);
		~ThresholdQueue();

		// From QueueWriter
		void* GetRawEnqueuePtr(ulong thresh, ulong chan=0);
		void Enqueue(ulong count);
		bool RawEnqueue(void* data, ulong count, ulong chan=0);
		ulong NumChannels(void) const;
		ulong Freespace(void) const;
		bool Full(void) const;

		// From QueueReader
		const void* GetRawDequeuePtr(ulong thresh, ulong chan=0);
		void Dequeue(ulong count);
		bool RawDequeue(void * data, ulong count, ulong chan=0);
		//ulong NumChannels(void) const;
		ulong Count(void) const;
		bool Empty(void) const;

		// From QueueBase
		ulong ElementsEnqueued(void) const;
		ulong ElementsDequeued(void) const;

		void RegisterReaderEvent(PthreadCondition* evt) {
			PthreadMutexProtected protectqlock(qlock);
			qwritten = evt;
		}

		void RegisterWriterEvent(PthreadCondition* evt) {
			PthreadMutexProtected protectqlock(qlock);
		       	qread = evt;
		}

		ulong ChannelStride(void) const;
	private:
		ThresholdQueueBase queue;
		mutable PthreadMutex qlock;
		PthreadCondition* qwritten;
		PthreadCondition* qread;
	};

}
#endif
