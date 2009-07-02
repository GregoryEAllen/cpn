/** \file
 * Implementation for the simple queue.
 *
 */

#include "SimpleQueue.h"
#include "QueueFactory.h"
#include "PthreadCondition.h"
#include "AutoLock.h"
#include <cassert>


CPN::SimpleQueue::SimpleQueue(const QueueAttr& attr)
       : CPN::QueueBase(attr), queueLength(attr.GetLength() + 1),
	maxThreshold(attr.GetMaxThreshold()), head(0), tail(0),
	qwritten(0), qread(0) {
	assert(attr.GetNumChannels() == 1);
	buffer.assign(queueLength + maxThreshold, '\0');
}

CPN::SimpleQueue::~SimpleQueue() {
}


// From QueueWriter
void* CPN::SimpleQueue::GetRawEnqueuePtr(CPN::ulong thresh, CPN::ulong chan) {
	assert(chan == 0);
	Sync::AutoLock l(qlock);
	if (Freespace() >= thresh) {
		return &buffer[head];
	} else {
		return 0;
	}
}

void CPN::SimpleQueue::Enqueue(CPN::ulong count) {
	Sync::AutoLock l(qlock);
	unsigned long newHead = head + count;
	if (newHead >= queueLength) {
		newHead -= queueLength;
		memcpy(&buffer[0], &buffer[queueLength], newHead);
	}
	head = newHead;
	if (qwritten) qwritten->Signal();
}

bool CPN::SimpleQueue::RawEnqueue(void* data, CPN::ulong count, CPN::ulong chan) {
	assert(chan == 0);
	Sync::AutoLock l(qlock);
	void* dest = GetRawEnqueuePtr(count, chan);
	if (dest) {
		memcpy(dest, data, count);
		Enqueue(count);
		return true;
	} else {
		return false;
	}
}

CPN::ulong CPN::SimpleQueue::NumChannels(void) const {
	return 1;
}

CPN::ulong CPN::SimpleQueue::Freespace(void) const {
	Sync::AutoLock l(qlock);
	if (head >= tail) {
		return queueLength - (head - tail) - 1;
	} else {
		return tail - head - 1;
	}
}

bool CPN::SimpleQueue::Full(void) const {
	return Freespace() == 0;
}

// From QueueReader
const void* CPN::SimpleQueue::GetRawDequeuePtr(CPN::ulong thresh, CPN::ulong chan) {
	assert(chan == 0);
	Sync::AutoLock l(qlock);
	if (Count() >= thresh) {
		if (tail + thresh > queueLength) {
			memcpy(&buffer[queueLength], &buffer[0], tail + thresh - queueLength);
		}
		return &buffer[tail];
	} else {
		return 0;
	}
}

void CPN::SimpleQueue::Dequeue(CPN::ulong count) {
	Sync::AutoLock l(qlock);
	unsigned long newTail = tail + count;
	if (newTail >= queueLength) {
		newTail -= queueLength;
	}
	tail = newTail;
	if (qread) qread->Signal();
}

bool CPN::SimpleQueue::RawDequeue(void * data, CPN::ulong count, CPN::ulong chan) {
	assert(chan == 0);
	Sync::AutoLock l(qlock);
	const void* src = GetRawDequeuePtr(count, chan);
	if (src) {
		memcpy(data, src, count);
		Dequeue(count);
		return true;
	} else {
		return false;
	}
}

CPN::ulong CPN::SimpleQueue::Count(void) const {
	Sync::AutoLock l(qlock);
	if (head >= tail) {
		return head - tail;
	} else {
		return head + (queueLength - tail);
	}
}

bool CPN::SimpleQueue::Empty(void) const {
	Sync::AutoLock l(qlock);
	return head == tail;
}

// From QueueBase
CPN::ulong CPN::SimpleQueue::ElementsEnqueued(void) const {
	return 0;
}
CPN::ulong CPN::SimpleQueue::ElementsDequeued(void) const {
	return 0;
}

void CPN::SimpleQueue::RegisterReaderEvent(PthreadCondition* evt) {
	Sync::AutoLock l(qlock);
	qwritten = evt;
	if (qwritten) qwritten->Signal();
}

void CPN::SimpleQueue::RegisterWriterEvent(PthreadCondition* evt) {
	Sync::AutoLock l(qlock);
	qread = evt;
	if (qread) qread->Signal();
}

class SQFactory : public CPN::QueueFactory {
public:
	SQFactory() : CPN::QueueFactory(CPN_QUEUETYPE_SIMPLE) {}

	CPN::QueueBase* Create(const CPN::QueueAttr& attr) {
		return new CPN::SimpleQueue(attr);
	}

	void Destroy(CPN::QueueBase* queue) {
		delete queue;
	}
};

static SQFactory factoryInstance;
void CPN::SimpleQueue::RegisterQueueType(void) {
	CPNRegisterQueueFactory(&factoryInstance);
}

