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
    maxThreshold(attr.GetMaxThreshold()), numChannels(attr.GetNumChannels()),
    head(0), tail(0) {
    buffer.assign((queueLength + maxThreshold)*numChannels, '\0');
}

CPN::SimpleQueue::~SimpleQueue() {
}


// From QueueWriter
void* CPN::SimpleQueue::GetRawEnqueuePtr(CPN::ulong thresh, CPN::ulong chan) {
    Sync::AutoLock l(qlock);
    assert(chan < numChannels);
    if (Freespace() >= thresh) {
        return &buffer[head + (queueLength + maxThreshold)*chan];
    } else {
        return 0;
    }
}

void CPN::SimpleQueue::Enqueue(CPN::ulong count) {
    Sync::AutoLock l(qlock);
    unsigned long newHead = head + count;
    if (newHead >= queueLength) {
        newHead -= queueLength;
        for (unsigned long chan = 0; chan < numChannels; ++chan) {
            unsigned long chanOff = (queueLength + maxThreshold)*chan;
            memcpy(&buffer[0 + chanOff], &buffer[queueLength + chanOff], newHead);
        }
    }
    head = newHead;
    NotifyReaderOfWrite();
}

bool CPN::SimpleQueue::RawEnqueue(void* data, CPN::ulong count) {
    Sync::AutoLock l(qlock);
    void* dest = GetRawEnqueuePtr(count);
    if (dest) {
        memcpy(dest, data, count);
        Enqueue(count);
        return true;
    } else {
        return false;
    }
}

bool CPN::SimpleQueue::RawEnqueue(void* data, CPN::ulong count,
        CPN::ulong numChans, CPN::ulong chanStride) {
    Sync::AutoLock l(qlock);
    void* dest = GetRawEnqueuePtr(count, 0);
    void* src = data;
    if (!dest) { return false; }
    memcpy(dest, src, count);
    for (unsigned long chan = 0; chan < numChans; ++chan) {
        dest = GetRawEnqueuePtr(count, chan);
        src = ((char*)data) + (queueLength + maxThreshold)*chan;
        memcpy(dest, src, count);
    }
    Enqueue(count);
    return true;
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
    Sync::AutoLock l(qlock);
    unsigned long chanOff = (queueLength + maxThreshold)*chan;
    if (Count() >= thresh) {
        if (tail + thresh > queueLength) {
            memcpy(&buffer[queueLength + chanOff], &buffer[0 + chanOff],
                    tail + thresh - queueLength);
        }
        return &buffer[tail + chanOff];
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
    NotifyWriterOfRead();
}

bool CPN::SimpleQueue::RawDequeue(void * data, CPN::ulong count) {
    Sync::AutoLock l(qlock);
    const void* src = GetRawDequeuePtr(count);
    if (src) {
        memcpy(data, src, count);
        Dequeue(count);
        return true;
    } else {
        return false;
    }
}

bool CPN::SimpleQueue::RawDequeue(void * data, CPN::ulong count,
        ulong numChans, ulong chanStride) {
    Sync::AutoLock l(qlock);
    const void* src = GetRawDequeuePtr(count, 0);
    void* dest = data;
    if (!src) { return false; }
    memcpy(dest, src, count);
    for (unsigned long chan = 0; chan < numChannels; ++chan) {
        src = GetRawDequeuePtr(count, chan);
        dest = ((char*)data) + (queueLength + maxThreshold)*chan;
        memcpy(dest, src, count);
    }
    Dequeue(count);
    return true;
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

