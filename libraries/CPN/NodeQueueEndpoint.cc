/** \file
 */

#include "NodeQueueEndpoint.h"
#include "KernelShutdownException.h"
#include "AutoLock.h"
#include <cassert>


CPN::NodeQueueEndpoint::NodeQueueEndpoint(NodeInfo* nodeinfo_, const std::string &portname_)
	: nodeinfo(nodeinfo_), portname(portname_), lock(), queueinfo(0),
	status(CPN::QueueStatus::DETACHED, &lock) {
}

CPN::NodeQueueEndpoint::~NodeQueueEndpoint() {
	assert(queueinfo == 0);
}

void CPN::NodeQueueEndpoint::Terminate(void) {
	Sync::AutoLock alock(lock);
	status.Post(QueueStatus::SHUTDOWN);
}

void CPN::NodeQueueEndpoint::SetQueueInfo(QueueInfo* queueinfo_) {
	Sync::AutoLock alock(lock);
	if (queueinfo == queueinfo_) return;
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
			ClearQueueInfoEndpoint();
			queueinfo = queueinfo_;
			if (queueinfo) {
				SetQueueInfoEndpoint();
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

void CPN::NodeQueueEndpoint::ClearQueueInfo() {
	SetQueueInfo(0);
}

CPN::QueueInfo* CPN::NodeQueueEndpoint::GetQueueInfo(void) {
	Sync::AutoLock alock(lock);
	return queueinfo;
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
CPN::QueueBase* CPN::NodeQueueEndpoint::CheckQueue(void) const {
	QueueStatus newStatus = status.CompareAndWait(QueueStatus::DETACHED);
	if (newStatus == QueueStatus::SHUTDOWN) {
		throw KernelShutdownException("Shutting down.");
	}
	return queueinfo->GetQueue();
}

CPN::QueueBase* CPN::NodeQueueEndpoint::StartOperation(void) {
	QueueBase* queue = CheckQueue();
	status.Post(QueueStatus::QUERY);
	return queue;
}

void CPN::NodeQueueEndpoint::Block(void) {
	status.ComparePostAndWait(QueueStatus::QUERY, QueueStatus::BLOCKED);
}

void CPN::NodeQueueEndpoint::ContinueOperation(void) {
	status.Post(QueueStatus::TRANSFER);
}

void CPN::NodeQueueEndpoint::CompleteOperation(void) {
	status.Post(QueueStatus::READY);
}

