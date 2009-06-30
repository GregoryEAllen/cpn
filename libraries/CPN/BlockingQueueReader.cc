/** \file
 */

#include "BlockingQueueReader.h"
#include "QueueBase.h"
#include "KernelShutdownException.h"
#include <cassert>


const void* CPN::BlockingQueueReader::GetRawDequeuePtr(ulong thresh, ulong chan) {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	const void* ptr = queue->GetRawDequeuePtr(thresh, chan);
	while (!ptr) {
		event.Wait(lock);
		queue = CheckQueue();
		ptr = queue->GetRawDequeuePtr(thresh, chan);
	}
	outstandingDequeue = true;
	return ptr;
}
void CPN::BlockingQueueReader::Dequeue(ulong count) {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	assert(outstandingDequeue);
	queue->Dequeue(count);
	outstandingDequeue = false;
	swapQueueEvent.Signal();
}

bool CPN::BlockingQueueReader::RawDequeue(void * data, ulong count, ulong chan) {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	assert(false == outstandingDequeue);
	while (!queue->RawDequeue(data, count, chan)) {
		event.Wait(lock);
		queue = CheckQueue();
	}
	return true;
}

CPN::ulong CPN::BlockingQueueReader::NumChannels(void) const {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	return ((QueueReader*)queue)->NumChannels();
}
CPN::ulong CPN::BlockingQueueReader::Count(void) const {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	return queue->Count();
}
bool CPN::BlockingQueueReader::Empty(void) const {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	return queue->Empty();
}

void CPN::BlockingQueueReader::SetQueueInfo(QueueInfo* queueinfo_) {
	PthreadMutexProtected protectlock(lock);
	while (outstandingDequeue && !shutdown) { swapQueueEvent.Wait(lock); }
	queueinfo = queueinfo_;
	event.Signal();
}

CPN::QueueInfo* CPN::BlockingQueueReader::GetQueueInfo(void) {
	PthreadMutexProtected protectlock(lock);
	return queueinfo;
}

void CPN::BlockingQueueReader::Terminate(void) {
	PthreadMutexProtected protectlock(lock);
	shutdown = true;
	event.Signal();
	swapQueueEvent.Signal();
}

CPN::QueueBase* CPN::BlockingQueueReader::CheckQueue(void) const {
	while (!queueinfo && !shutdown) { event.Wait(lock); }
	if (shutdown) throw CPN::KernelShutdownException("Shutting down.");
	return queueinfo->GetQueue();
}



