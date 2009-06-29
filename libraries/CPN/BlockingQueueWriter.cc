/** \file
 */

#include "BlockingQueueWriter.h"
#include "QueueBase.h"
#include "KernelShutdownException.h"

void* CPN::BlockingQueueWriter::GetRawEnqueuePtr(ulong thresh, ulong chan) {
	PthreadMutexProtected protectlock(lock);
	QueueBase* queue = CheckQueue();
	void* ptr = queue->GetRawEnqueuePtr(thresh, chan);
	while (!ptr) {
		event.Wait(lock);
		queue = CheckQueue();
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
		queue = CheckQueue();
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

void CPN::BlockingQueueWriter::SetQueueInfo(QueueInfo* queueinfo_) {
	PthreadMutexProtected protectlock(lock);
	queueinfo = queueinfo_;
	event.Signal();
}

CPN::QueueInfo* CPN::BlockingQueueWriter::GetQueueInfo(void) {
	PthreadMutexProtected protectlock(lock);
	return queueinfo;
}

void CPN::BlockingQueueWriter::Terminate(void) {
	PthreadMutexProtected protectlock(lock);
	shutdown = true;
	event.Signal();
}


CPN::QueueBase* CPN::BlockingQueueWriter::CheckQueue(void) const {
	while (!queueinfo) {
		event.Wait(lock);
		if (shutdown) throw CPN::KernelShutdownException("Shutting down.");
	}
	return queueinfo->GetQueue();
};






