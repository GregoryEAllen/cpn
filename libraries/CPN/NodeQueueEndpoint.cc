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
	assert(queueinfo_);
	assert(0 == queueinfo);
	QueueStatus theStatus = status.Get();
	while (true) {
		switch (theStatus.status) {
		case QueueStatus::DETACHED:
			queueinfo = queueinfo_;
			SetQueueInfoEndpoint(queueinfo);
			status.Post(QueueStatus::READY);
			return;
		case QueueStatus::SHUTDOWN:
			queueinfo = queueinfo_;
			return;
		case QueueStatus::QUERY:
		case QueueStatus::TRANSFER:
		case QueueStatus::BLOCKED:
		case QueueStatus::READY:
		default:
			// We cannot be in any of these states.
			assert(false);
		}
	}
}

void CPN::NodeQueueEndpoint::ClearQueueInfo(bool checkdeath) {
	Sync::AutoLock alock(lock);
	if (!queueinfo) return;
	QueueStatus theStatus = status.Get();
	QueueInfo* qinfo;
	while (true) {
		switch (theStatus.status) {
		case QueueStatus::QUERY:
		case QueueStatus::TRANSFER:
			theStatus = status.CompareAndWait(theStatus);
			break;
		case QueueStatus::BLOCKED:
		case QueueStatus::READY:
			status.Post(QueueStatus::DETACHED);
		case QueueStatus::DETACHED:
		case QueueStatus::SHUTDOWN:
			qinfo = queueinfo;
			queueinfo = 0;
			ClearQueueInfoEndpoint(qinfo, checkdeath);
			return;
		default:
			assert(false);
		}
	}
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

