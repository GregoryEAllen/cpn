/** \file
 */

#include "BlockingQueueWriter.h"
#include "QueueBase.h"
#include "AutoLock.h"
#include <cassert>

CPN::BlockingQueueWriter::BlockingQueueWriter(NodeInfo* nodeinfo, const std::string &portname)
	: NodeQueueWriter(nodeinfo, portname) {}

CPN::BlockingQueueWriter::~BlockingQueueWriter() {
}

void* CPN::BlockingQueueWriter::GetRawEnqueuePtr(ulong thresh, ulong chan) {
	Sync::AutoLock alock(lock);
	QueueBase* queue = StartOperation();
	void* ptr = queue->GetRawEnqueuePtr(thresh, chan);
	while (!ptr) {
		Block();
		queue = StartOperation();
		ptr = queue->GetRawEnqueuePtr(thresh, chan);
	}
	ContinueOperation();
	return ptr;
}

void CPN::BlockingQueueWriter::Enqueue(ulong count) {
	Sync::AutoLock alock(lock);
	QueueBase* queue = CheckQueue();
	queue->Enqueue(count);
	CompleteOperation();
}

bool CPN::BlockingQueueWriter::RawEnqueue(void* data, ulong count, ulong chan) {
	Sync::AutoLock alock(lock);
	QueueBase* queue = StartOperation();
	while (!queue->RawEnqueue(data, count, chan)) {
		Block();
		queue =	StartOperation();
	}
	CompleteOperation();
	return true;
}

CPN::ulong CPN::BlockingQueueWriter::NumChannels(void) const {
	Sync::AutoLock alock(lock);
	QueueBase* queue = CheckQueue();
	return ((QueueWriter*)queue)->NumChannels();
}
CPN::ulong CPN::BlockingQueueWriter::Freespace(void) const {
	Sync::AutoLock alock(lock);
	QueueBase* queue = CheckQueue();
	return queue->Freespace();
}
bool CPN::BlockingQueueWriter::Full(void) const {
	Sync::AutoLock alock(lock);
	QueueBase* queue = CheckQueue();
	return queue->Full();
}

const CPN::QueueDatatype* CPN::BlockingQueueWriter::GetDatatype(void) const {
	Sync::AutoLock alock(lock);
	QueueBase* queue = CheckQueue();
	return queue->GetDatatype();
}

