/** \file
 */

#include "BlockingQueueReader.h"
#include "QueueBase.h"
#include "AutoLock.h"
#include <cassert>

CPN::BlockingQueueReader::BlockingQueueReader(NodeInfo* nodeinfo, const std::string &portname)
	: NodeQueueReader(nodeinfo, portname) {}

CPN::BlockingQueueReader::~BlockingQueueReader() {
}

const void* CPN::BlockingQueueReader::GetRawDequeuePtr(ulong thresh, ulong chan) {
	Sync::AutoLock alock(lock);
	QueueBase* queue = StartOperation();
	const void* ptr = queue->GetRawDequeuePtr(thresh, chan);
	while (!ptr) {
		Block();
		queue = StartOperation();
		ptr = queue->GetRawDequeuePtr(thresh, chan);
	}
	ContinueOperation();
	return ptr;
}

void CPN::BlockingQueueReader::Dequeue(ulong count) {
	Sync::AutoLock alock(lock);
	QueueBase* queue = CheckQueue();
	queue->Dequeue(count);
	CompleteOperation();
}

bool CPN::BlockingQueueReader::RawDequeue(void *data, ulong count, ulong chan) {
	Sync::AutoLock alock(lock);
	QueueBase* queue = StartOperation();
	while (!queue->RawDequeue(data, count, chan)) {
		Block();
		queue =	StartOperation();
	}
	CompleteOperation();
	return true;
}

CPN::ulong CPN::BlockingQueueReader::NumChannels(void) const {
	Sync::AutoLock alock(lock);
	QueueBase* queue = CheckQueue();
	return ((QueueReader*)queue)->NumChannels();
}

CPN::ulong CPN::BlockingQueueReader::Count(void) const {
	Sync::AutoLock alock(lock);
	QueueBase* queue = CheckQueue();
	return queue->Count();
}

bool CPN::BlockingQueueReader::Empty(void) const {
	Sync::AutoLock alock(lock);
	QueueBase* queue = CheckQueue();
	return queue->Empty();
}

