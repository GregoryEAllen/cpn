/** \file
 */

#include "BlockingQueueReader.h"
#include "QueueBase.h"
#include "KernelShutdownException.h"
#include "AutoLock.h"
#include <cassert>

CPN::BlockingQueueReader::BlockingQueueReader(NodeInfo* nodeinfo, const std::string &portname)
	: NodeQueueReader(nodeinfo, portname), lock(), queueinfo(0),
	status(CPN::QueueStatus::DETACHED, lock) {}

const void* CPN::BlockingQueueReader::GetRawDequeuePtr(ulong thresh, ulong chan) {
	Sync::AutoLock alock(lock);
	QueueBase* queue = CheckQueue();
	status.Post(QueueStatus::QUERY);
	const void* ptr = queue->GetRawDequeuePtr(thresh, chan);
	while (!ptr) {
		status.ComparePostAndWait(QueueStatus::QUERY, QueueStatus::BLOCKED);
		queue = CheckQueue();
		status.Post(QueueStatus::QUERY);
		ptr = queue->GetRawDequeuePtr(thresh, chan);
	}
	status.Post(QueueStatus::TRANSFER);
	return ptr;
}

void CPN::BlockingQueueReader::Dequeue(ulong count) {
	Sync::AutoLock alock(lock);
	QueueBase* queue = CheckQueue();
	queue->Dequeue(count);
	status.Post(QueueStatus::READY);
}

bool CPN::BlockingQueueReader::RawDequeue(void * data, ulong count, ulong chan) {
	Sync::AutoLock alock(lock);
	QueueBase* queue = CheckQueue();
	status.Post(QueueStatus::QUERY);
	while (!queue->RawDequeue(data, count, chan)) {
		status.ComparePostAndWait(QueueStatus::QUERY, QueueStatus::BLOCKED);
		queue = CheckQueue();
		status.Post(QueueStatus::QUERY);
	}
	status.Post(QueueStatus::READY);
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

void CPN::BlockingQueueReader::SetQueueInfo(QueueInfo* queueinfo_) {
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
				queueinfo->GetQueue()->SetReaderStatusHandler(0);
			}
			queueinfo = queueinfo_;
			if (queueinfo) {
				queueinfo->GetQueue()->SetReaderStatusHandler(&status);
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

CPN::QueueInfo* CPN::BlockingQueueReader::GetQueueInfo(void) {
	Sync::AutoLock alock(lock);
	return queueinfo;
}

void CPN::BlockingQueueReader::Terminate(void) {
	Sync::AutoLock alock(lock);
	status.Post(QueueStatus::SHUTDOWN);
}

/**
 * Block if we are detached and throw the KernelShutdownException
 * if we are shutdown.
 *
 * The only states that we should be in for this function are:
 * TRANSFER, READY, DETACHED, and SHUTDOWN
 *
 * This function will only every exit normally on the states:
 * TRANSFER, READY
 *
 * \return the QueueBase that is attached to us
 */
CPN::QueueBase* CPN::BlockingQueueReader::CheckQueue(void) const {
	QueueStatus newStatus = status.CompareAndWait(QueueStatus::DETACHED);
	if (newStatus == QueueStatus::SHUTDOWN) {
		throw KernelShutdownException("Shutting down.");
	}
	return queueinfo->GetQueue();
}

