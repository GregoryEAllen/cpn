/** \file
 */

#include "BlockingQueueWriter.h"
#include "QueueBase.h"
#include "KernelShutdownException.h"
#include <cassert>

CPN::BlockingQueueWriter::BlockingQueueWriter(NodeInfo* nodeinfo, const std::string &portname)
	: NodeQueueWriter(nodeinfo, portname),lock(), queueinfo(0),
	status(CPN::QueueStatus::DETACHED, lock) {}

void* CPN::BlockingQueueWriter::GetRawEnqueuePtr(ulong thresh, ulong chan) {
	QueueBase* queue = CheckQueue();
	status.Post(QueueStatus::QUERY);
	void* ptr = queue->GetRawEnqueuePtr(thresh, chan);
	while (!ptr) {
		status.ComparePostAndWait(QueueStatus::QUERY, QueueStatus::BLOCKED);
		queue = CheckQueue();
		status.Post(QueueStatus::QUERY);
		ptr = queue->GetRawEnqueuePtr(thresh, chan);
	}
	status.Post(QueueStatus::TRANSFER);
	return ptr;
}

void CPN::BlockingQueueWriter::Enqueue(ulong count) {
	Sync::AutoLock alock(lock);
	QueueBase* queue = CheckQueue();
	queue->Enqueue(count);
	status.Post(QueueStatus::READY);
}

bool CPN::BlockingQueueWriter::RawEnqueue(void* data, ulong count, ulong chan) {
	Sync::AutoLock alock(lock);
	QueueBase* queue = CheckQueue();
	status.Post(QueueStatus::QUERY);
	while (!queue->RawEnqueue(data, count, chan)) {
		status.ComparePostAndWait(QueueStatus::QUERY, QueueStatus::BLOCKED);
		queue = CheckQueue();
		status.Post(QueueStatus::QUERY);
	}
	status.Post(QueueStatus::READY);
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

void CPN::BlockingQueueWriter::SetQueueInfo(QueueInfo* queueinfo_) {
	Sync::AutoLock alock(lock);
	QueueStatus theStatus = status.Get();
	while (true) {
		switch (theStatus.status) {
		case QueueStatus::QUERY:
		case QueueStatus::TRANSFER:
			theStatus = status.CompareAndWait(theStatus);
			break;
		case QueueStatus::BLOCKED:
		case QueueStatus::READY:
		case QueueStatus::DETACHED:
			if (queueinfo) {
				queueinfo->GetQueue()->SetWriterStatusHandler(0);
			}
			queueinfo = queueinfo_;
			if (queueinfo) {
				queueinfo->GetQueue()->SetWriterStatusHandler(&status);
				status.Post(QueueStatus::READY);
			} else {
				status.Post(QueueStatus::DETACHED);
			}
			return;
		case QueueStatus::SHUTDOWN:
			queueinfo = queueinfo_;
			return;
		default:
			assert(false);
		}
	}
}

CPN::QueueInfo* CPN::BlockingQueueWriter::GetQueueInfo(void) {
	Sync::AutoLock alock(lock);
	return queueinfo;
}

void CPN::BlockingQueueWriter::Terminate(void) {
	Sync::AutoLock alock(lock);
	status.Post(QueueStatus::SHUTDOWN);
}


CPN::QueueBase* CPN::BlockingQueueWriter::CheckQueue(void) const {
	QueueStatus newStatus = status.CompareAndWait(QueueStatus::DETACHED);
	if (newStatus == QueueStatus::SHUTDOWN) {
		throw KernelShutdownException("Shutting down.");
	}
	return queueinfo->GetQueue();
};






