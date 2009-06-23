/** \file
 */

#include "BlockingQueueReader.h"
#include "QueueBase.h"


const void* CPN::BlockingQueueReader::GetRawDequeuePtr(ulong thresh, ulong chan) {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	const void* ptr = queue->GetRawDequeuePtr(thresh, chan);
	while (!ptr) {
		event.Wait(lock);
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

void CPN::BlockingQueueReader::SetQueue(QueueInfo* queueinfo_) {
	PthreadMutexProtected protectlock(lock);
	if (queueinfo) queueinfo->SetReader(0);
	queueinfo = queueinfo_;
	if (queueinfo) queueinfo->SetReader(this);
	event.Signal();
}

CPN::QueueInfo* CPN::BlockingQueueReader::GetQueue(void) {
	PthreadMutexProtected protectlock(lock);
	return queueinfo;
}

