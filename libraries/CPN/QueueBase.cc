/** \file
 */
#include "QueueBase.h"

CPN::QueueBase::QueueBase(const QueueAttr &qattr) 
	: qattr(qattr), readerStatusHandler(0), writerStatusHandler(0) {}
CPN::QueueBase::~QueueBase() {}

void CPN::QueueBase::SetReaderStatusHandler(Sync::StatusHandler<CPN::QueueStatus>* rsh) {
	PthreadMutexProtected p(statusHandlerMutex);
	readerStatusHandler = rsh;
}
void CPN::QueueBase::SetWriterStatusHandler(Sync::StatusHandler<CPN::QueueStatus>* wsh) {
	PthreadMutexProtected p(statusHandlerMutex);
	writerStatusHandler = wsh;
}

void CPN::QueueBase::NotifyReaderOfWrite(void) {
	PthreadMutexProtected p(statusHandlerMutex);
	if (readerStatusHandler) {
		Notify(readerStatusHandler, QueueStatus::TRANSFER);
	}
}

void CPN::QueueBase::NotifyWriterOfRead(void) {
	PthreadMutexProtected p(statusHandlerMutex);
	if (writerStatusHandler) {
		Notify(writerStatusHandler, QueueStatus::TRANSFER);
	}
}

void CPN::QueueBase::Notify(Sync::StatusHandler<QueueStatus>* stathand,
	CPN::QueueStatus newStatus) {
	while (true) {
		QueueStatus oldStatus = stathand->Get();
		switch (oldStatus.status) {
		case QueueStatus::QUERY:
		case QueueStatus::BLOCKED:
			if (stathand->CompareAndPost(oldStatus, newStatus)) {
				return;
			}
			break;
		default:
			return;
		}
	}
}
