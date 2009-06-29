/** \file
 */

#include "BlockingQueueReader.h"
#include "QueueBase.h"
#include "KernelShutdownException.h"


const void* CPN::BlockingQueueReader::GetRawDequeuePtr(ulong thresh, ulong chan) {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	const void* ptr = queue->GetRawDequeuePtr(thresh, chan);
	while (!ptr) {
		event.Wait(lock);
		queue = CheckQueue();
		ptr = queue->GetRawDequeuePtr(thresh, chan);
	}
	return ptr;
}
void CPN::BlockingQueueReader::Dequeue(ulong count) {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	queue->Dequeue(count);
}

bool CPN::BlockingQueueReader::RawDequeue(void * data, ulong count, ulong chan) {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
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
}

CPN::QueueBase* CPN::BlockingQueueReader::CheckQueue(void) const {
	while (!queueinfo) {
		event.Wait(lock);
		if (shutdown) throw CPN::KernelShutdownException("Shutting down.");
	}
	return queueinfo->GetQueue();
}



