/** \file
 * \brief BlockingQueueWriter
 */
#ifndef CPN_BLOCKINGQUEUEWRITER_H
#define CPN_BLOCKINGQUEUEWRITER_H

#include "QueueWriter.h"
#include "QueueBase.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"

namespace CPN {

	class BlockingQueueWriter : public QueueWriter {
	public:
		BlockingQueueWriter() : queue(0) {}

		void* GetRawEnqueuePtr(ulong thresh, ulong chan=0) {
			PthreadMutexProtected protectlock(lock);
			CheckQueue();
			void* ptr = queue->GetRawEnqueuePtr(thresh, chan);
			while (!ptr) {
				event.Wait(lock);
				ptr = queue->GetRawEnqueuePtr(thresh, chan);
			}
			return ptr;
		}
		void Enqueue(ulong count) {
			PthreadMutexProtected protectlock(lock);
			CheckQueue();
			queue->Enqueue(count);
		}
		bool RawEnqueue(void* data, ulong count, ulong chan=0) {
			PthreadMutexProtected protectlock(lock);
			CheckQueue();
			while (!queue->RawEnqueue(data, count, chan)) {
				event.Wait(lock);
			}
			return true;

		}
		ulong NumChannels(void) const {
			PthreadMutexProtected protectlock(lock);
			CheckQueue();
			return ((QueueWriter*)queue)->NumChannels();
		}
		ulong Freespace(void) const {
			PthreadMutexProtected protectlock(lock);
			CheckQueue();
			return queue->Freespace();
		}
		bool Full(void) const {
			PthreadMutexProtected protectlock(lock);
			CheckQueue();
			return queue->Full();
		}

		void SetQueue(QueueBase* queue_) {
			PthreadMutexProtected protectlock(lock);
			if (queue) queue->RegisterWriterEvent(0);
			queue = queue_;
			if (queue) queue->RegisterWriterEvent(&event);
			event.Signal();
		}

	private:
		void CheckQueue(void) const {
			while (!queue) event.Wait(lock);
		};

		mutable PthreadCondition event;
		mutable PthreadMutex lock;
		QueueBase* queue;
	};
}

#endif

