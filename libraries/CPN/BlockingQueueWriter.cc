/** \file
 */

#include "BlockingQueueWriter.h"
#include "QueueBase.h"

void* CPN::BlockingQueueWriter::GetRawEnqueuePtr(ulong thresh, ulong chan) {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	void* ptr = queue->GetRawEnqueuePtr(thresh, chan);
	while (!ptr) {
		event.Wait(lock);
		ptr = queue->GetRawEnqueuePtr(thresh, chan);
	}
	return ptr;
}
void CPN::BlockingQueueWriter::Enqueue(ulong count) {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	queue->Enqueue(count);
}
bool CPN::BlockingQueueWriter::RawEnqueue(void* data, ulong count, ulong chan) {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	while (!queue->RawEnqueue(data, count, chan)) {
		event.Wait(lock);
	}
	return true;

}
CPN::ulong CPN::BlockingQueueWriter::NumChannels(void) const {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	return ((QueueWriter*)queue)->NumChannels();
}
CPN::ulong CPN::BlockingQueueWriter::Freespace(void) const {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	return queue->Freespace();
}
bool CPN::BlockingQueueWriter::Full(void) const {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	return queue->Full();
}

void CPN::BlockingQueueWriter::SetQueue(QueueInfo* queueinfo_) {
	PthreadMutexProtected protectlock(lock);
	if (queueinfo) queueinfo->SetWriter(0);
	queueinfo = queueinfo_;
	if (queueinfo) queueinfo->SetWriter(this);
	event.Signal();
}

CPN::QueueInfo* CPN::BlockingQueueWriter::GetQueue(void) {
	PthreadMutexProtected protectlock(lock);
	return queueinfo;
}

