/** \file
 * \brief BlockingQueueReader
 */
#ifndef CPN_BLOCKINGQUEUEREADER_H
#define CPN_BLOCKINGQUEUEREADER_H

#include "QueueReader.h"
#include "QueueBase.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"

namespace CPN {

	class BlockingQueueReader : public QueueReader {
	public:
		BlockingQueueReader() : queue(0) {}

		~BlockingQueueReader() {
			PthreadMutexProtected protectlock(lock);
			SetQueue(0);
		}

		// From QueueReader
		const void* GetRawDequeuePtr(ulong thresh, ulong chan=0) {
			PthreadMutexProtected protectlock(lock);
			CheckQueue();
			const void* ptr = queue->GetRawDequeuePtr(thresh, chan);
			while (!ptr) {
				event.Wait(lock);
				ptr = queue->GetRawDequeuePtr(thresh, chan);
			}
			return ptr;
		}

		void Dequeue(ulong count) {
			PthreadMutexProtected protectlock(lock);
			CheckQueue();
			queue->Dequeue(count);
		}

		bool RawDequeue(void * data, ulong count, ulong chan=0) {
			PthreadMutexProtected protectlock(lock);
			CheckQueue();
			while (!queue->RawDequeue(data, count, chan)) {
				event.Wait(lock);
			}
			return true;
		}

		ulong NumChannels(void) const {
			PthreadMutexProtected protectlock(lock);
			CheckQueue();
			return ((QueueReader*)queue)->NumChannels();
		}
		ulong Count(void) const {
			PthreadMutexProtected protectlock(lock);
			CheckQueue();
			return queue->Count();
		}
		bool Empty(void) const {
			PthreadMutexProtected protectlock(lock);
			CheckQueue();
			return queue->Empty();
		}

		void SetQueue(QueueBase* queue_) {
			PthreadMutexProtected protectlock(lock);
			if (queue) queue->RegisterReaderEvent(0);
			queue = queue_;
			if (queue) queue->RegisterReaderEvent(&event);
			event.Signal();
		}

	private:
		void CheckQueue(void) const {
			while (!queue) event.Wait(lock);
		}
		mutable PthreadCondition event;
		mutable PthreadMutex lock;
		QueueBase* queue;
	};
}

#endif

