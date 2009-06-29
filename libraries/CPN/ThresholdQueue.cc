/** \file
 * \brief Implementation of CPN ThresholdQueue
 */

#include "ThresholdQueue.h"
#include "QueueBase.h"


CPN::ThresholdQueue::ThresholdQueue(const QueueAttr &attr) :
	QueueBase(attr),
// ThresholdQueueBase(ulong elemSize, ulong queueLen, ulong maxThresh, ulong numChans=1);
	queue(1, attr.GetLength(), attr.GetMaxThreshold(), attr.GetNumChannels()),
	qwritten(0), qread(0)
{
}

CPN::ThresholdQueue::~ThresholdQueue() {
}

// From QueueWriter

void* CPN::ThresholdQueue::GetRawEnqueuePtr(ulong thresh, ulong chan) {
	PthreadMutexProtected protectqlock(qlock);
	return queue.GetRawEnqueuePtr(thresh, chan);
}

void CPN::ThresholdQueue::Enqueue(ulong count) {
	PthreadMutexProtected protectqlock(qlock);
	queue.Enqueue(count);
	if (qwritten) qwritten->Signal();
}

bool CPN::ThresholdQueue::RawEnqueue(void * data, ulong count, ulong chan) {
	PthreadMutexProtected protectqlock(qlock);
	void* dest = queue.GetRawEnqueuePtr(count, chan);
	if (!dest) return false;
	memcpy(dest, data, count);
	queue.Enqueue(count);
	if (qwritten) qwritten->Signal();
	return true;
}

CPN::ulong CPN::ThresholdQueue::NumChannels(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.NumChannels();
}

CPN::ulong CPN::ThresholdQueue::ChannelStride(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.ChannelStride();
}

CPN::ulong CPN::ThresholdQueue::Freespace(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.Freespace();
}

bool CPN::ThresholdQueue::Full(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.Full();
}


// From QueueReader
const void* CPN::ThresholdQueue::GetRawDequeuePtr(ulong thresh, ulong chan) {
	PthreadMutexProtected protectqlock(qlock);
	return queue.GetRawDequeuePtr(thresh, chan);
}

void CPN::ThresholdQueue::Dequeue(ulong count) {
	PthreadMutexProtected protectqlock(qlock);
	queue.Dequeue(count);
	if (qread) qread->Signal();
}

bool CPN::ThresholdQueue::RawDequeue(void * data, ulong count, ulong chan) {
	PthreadMutexProtected protectqlock(qlock);
	const void* src = queue.GetRawDequeuePtr(count, chan);
	if (!src) return false;
	memcpy(data, src, count);
	queue.Dequeue(count);
	if (qread) qread->Signal();
	return true;
}

CPN::ulong CPN::ThresholdQueue::Count(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.Count();
}

bool CPN::ThresholdQueue::Empty(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.Empty();
}


// From QueueBase

CPN::ulong CPN::ThresholdQueue::ElementsEnqueued(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.ElementsEnqueued();
}

CPN::ulong CPN::ThresholdQueue::ElementsDequeued(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.ElementsDequeued();
}

void CPN::ThresholdQueue::RegisterReaderEvent(PthreadCondition* evt) {
	PthreadMutexProtected protectqlock(qlock);
	qwritten = evt;
	if (qwritten) qwritten->Signal();
}

void CPN::ThresholdQueue::RegisterWriterEvent(PthreadCondition* evt) {
	PthreadMutexProtected protectqlock(qlock);
	qread = evt;
	if (qread) qread->Signal();
}

