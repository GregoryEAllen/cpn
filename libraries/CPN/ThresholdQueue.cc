/** \file
 * \brief Implementation of CPN ThresholdQueue
 */

#include "ThresholdQueue.h"
#include "QueueBase.h"
#include "ThresholdQueueFactory.h"


CPN::ThresholdQueue::ThresholdQueue(const QueueAttr &attr_) :
    QueueBase(attr_),
// ThresholdQueueBase(ulong elemSize, ulong queueLen, ulong maxThresh, ulong numChans=1);
    queue(1, attr.GetLength(), attr.GetMaxThreshold(), attr.GetNumChannels())
{
    attr.SetLength(queue.QueueLength());
    attr.SetMaxThreshold(queue.MaxThreshold());
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
    NotifyReaderOfWrite();
}

bool CPN::ThresholdQueue::RawEnqueue(void* data, ulong count) {
    PthreadMutexProtected protectqlock(qlock);
    void* dest = queue.GetRawEnqueuePtr(count);
    if (!dest) return false;
    memcpy(dest, data, count);
    queue.Enqueue(count);
    NotifyReaderOfWrite();
    return true;
}

bool CPN::ThresholdQueue::RawEnqueue(void* data, ulong count,
        ulong numChans, ulong chanStride) {
    PthreadMutexProtected protectqlock(qlock);
    void* dest = queue.GetRawEnqueuePtr(count, 0);
    void* src = data;
    if (!dest) return false;
    memcpy(dest, src, count);
    for (ulong i = 1; i < numChans; ++i) {
        dest = queue.GetRawEnqueuePtr(count, i);
        src = ((char*)data) + (chanStride*i);
        memcpy(dest, src, count);
    }
    queue.Enqueue(count);
    NotifyReaderOfWrite();
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
    NotifyWriterOfRead();
}

bool CPN::ThresholdQueue::RawDequeue(void* data, ulong count) {
    PthreadMutexProtected protectqlock(qlock);
    const void* src = queue.GetRawDequeuePtr(count);
    if (!src) return false;
    memcpy(data, src, count);
    queue.Dequeue(count);
    NotifyWriterOfRead();
    return true;
}

bool CPN::ThresholdQueue::RawDequeue(void* data, ulong count,
        ulong numChans, ulong chanStride) {
    PthreadMutexProtected protectqlock(qlock);
    const void* src = queue.GetRawDequeuePtr(count, 0);
    void* dest = data;
    if (!src) return false;
    memcpy(dest, src, count);
    for (ulong i = 1; i < numChans; ++i) {
        src = queue.GetRawDequeuePtr(count, i);
        dest = ((char*)data) + (chanStride*i);
        memcpy(dest, src, count);
    }
    queue.Dequeue(count);
    NotifyWriterOfRead();
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

void CPN::ThresholdQueue::RegisterQueueType(void) {
    CPNRegisterQueueFactory(ThresholdQueueFactory::GetInstance());
}


