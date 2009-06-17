/** \file
 * \brief Implementation of CPN ThresholdQueue
 */

#include "ThresholdQueue.h"

CPN::ThresholdQueue::ThresholdQueue(ulong queueLen, ulong maxThresh, ulong numChans=1) :
// ThresholdQueueBase(ulong elemSize, ulong queueLen, ulong maxThresh, ulong numChans=1);
	queue(1, queueLen, maxThresh, numChans)
{
}

CPN::ThresholdQueue::~ThresholdQueue() {
}

// From QueueWriter

void* CPN::ThresholdQueue::GetRawEnqueuePtr(ulong thresh, ulong chan=0) {
	PthreadMutexProtected protectqlock(qlock);
	void* ptr = queue.GetRawEnqueuePtr(thresh, chan);
	while (0 == ptr) {
		qread.Wait(qlock);
		ptr = queue.GetRawEnqueuePtr(thresh, chan);
	}
	return ptr;
}

void CPN::ThresholdQueue::Enqueue(ulong count) {
	PthreadMutexProtected protectqlock(qlock);
	queue.Enqueue(count);
	qwritten.Signal();
}

ulong CPN::ThresholdQueue::NumChannels(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.NumChannels();
}

ulong CPN::ThresholdQueue::ChannelStride(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.ChannelStride();
}

ulong CPN::ThresholdQueue::Freespace(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.Freespace();
}

bool CPN::ThresholdQueue::Full(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.Full();
}


// From QueueReader
const void* CPN::ThresholdQueue::GetRawDequeuePtr(ulong thresh, ulong chan=0) {
	PthreadMutexProtected protectqlock(qlock);
	void* ptr = queue.GetRawDequeuePtr(thresh, chan);
	while (0 == ptr) {
		qwritten.Wait(qlock);
		ptr = queue.GetRawDequeuePtr(thresh, chan);
	}
	return ptr;
}

void CPN::ThresholdQueue::Dequeue(ulong count) {
	PthreadMutexProtected protectqlock(qlock);
	queue.Dequeue(count);
	qread.Signal();
}

ulong CPN::ThresholdQueue::NumChannels(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.NumChannels();
}

ulong CPN::ThresholdQueue::ChannelStride(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.ChannelStride();
}

ulong CPN::ThresholdQueue::Count(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.Count();
}

bool CPN::ThresholdQueue::Empty(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.Empty();
}


// From QueueBase

QueueWriter *CPN::ThresholdQueue::getWriter() {
	return this;
}

QueueReader *CPN::ThresholdQueue::getReader() {
	return this;
}

ulong CPN::ThresholdQueue::ElementsEnqueued(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.ElementsEnqueued();
}

ulong CPN::ThresholdQueue::ElementsDequeued(void) const {
	PthreadMutexProtected protectqlock(qlock);
	return queue.ElementsDequeued();
}


