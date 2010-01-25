
#include "QueueTest.h"

#include "QueueBase.h"
#include "QueueAttr.h"
#include "ThresholdQueue.h"
#include "SimpleQueue.h"

#include "MockDatabase.h"

#include "PthreadFunctional.h"

#include <stdlib.h>
#include <cppunit/TestAssert.h>
#include <vector>
#include <deque>

CPPUNIT_TEST_SUITE_REGISTRATION( QueueTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

using CPN::shared_ptr;
using CPN::QueueBase;
using CPN::ThresholdQueue;
using CPN::SimpleQueue;
using CPN::Database;
using CPN::SimpleQueueAttr;
using CPN::Key_t;

const char data[] = { 'a', 'b', 'c', 'd', 'e' };
const unsigned data_size = sizeof(data);
const Key_t RKEY = 1;
const Key_t WKEY = 2;

void *QueueTest::EnqueueData() {
    try {
        while (true) {
            for (unsigned chan = 0; chan < queue->NumChannels(); ++chan) {
                void *ptr = queue->GetRawEnqueuePtr(data_size, chan);
                memcpy(ptr, data, data_size);
            }
            queue->Enqueue(data_size);
            {
                PthreadMutexProtected al(enqueue_lock);
                enqueue_num += data_size;
                enqueue_cond.Signal();
                if (enqueue_stop) {
                    queue->ShutdownWriter();
                    break;
                }
            }
        }
    } catch (...) {
        PthreadMutexProtected al(enqueue_lock);
        enqueue_fail = true;
    }
    PthreadMutexProtected al(enqueue_lock);
    enqueue_dead = true;
    enqueue_cond.Signal();
    return 0;
}

void *QueueTest::DequeueData() {
    try {
        while (true) {
            bool fail = false;
            const void *ptr = queue->GetRawDequeuePtr(data_size, 0);
            if (!ptr) { break; }
            if (memcmp(ptr, data, data_size) != 0) { fail = true; }
            for (unsigned chan = 1; chan < queue->NumChannels() && !fail; ++chan) {
                ptr = queue->GetRawDequeuePtr(data_size, chan);
                if (memcmp(ptr, data, data_size) != 0) { fail = true; }
            }
            if (!fail) {
                queue->Dequeue(data_size);
                PthreadMutexProtected al(dequeue_lock);
                dequeue_num += data_size;
                dequeue_cond.Signal();
                if (dequeue_stop) {
                    queue->ShutdownReader();
                    break;
                }
            } else {
                PthreadMutexProtected al(dequeue_lock);
                dequeue_fail = true;
                break;
            }
        }
    } catch (...) {
        PthreadMutexProtected al(dequeue_lock);
        dequeue_fail = true;
    }
    PthreadMutexProtected al(dequeue_lock);
    dequeue_dead = true;
    dequeue_cond.Signal();
    return 0;
}

void QueueTest::Reset() {
    enqueue_fail = false;
    enqueue_stop = false;
    enqueue_dead = false;
    enqueue_num = 0;
    dequeue_fail = false;
    dequeue_stop = false;
    dequeue_dead = false;
    dequeue_num = 0;

}
void QueueTest::setUp() {
    Reset();
    queue = 0;
}

void QueueTest::tearDown() {
    delete queue;
    queue = 0;
}

// I want to first test that normal dequeue and enqueue work.
// Then I want to test that blocking works right on both sides
// Then I want to test that requesting a threshold larger than max thresh will
// grow the queue automatically
// Then I want to test growing the queue with and without pending dequeues and enqueues



void QueueTest::SimpleQueueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    shared_ptr<Database> database = shared_ptr<Database>(new MockDatabase);
    SimpleQueueAttr attr;
    attr.SetLength(309).SetMaxThreshold(10).SetNumChannels(1)
        .SetReaderKey(RKEY).SetWriterKey(WKEY);
	//DEBUG("%s : Size %u, MaxThresh %u, Chans %u\n",__PRETTY_FUNCTION__, attr.GetLength(), attr.GetMaxThreshold(), attr.GetNumChannels());
    queue = new SimpleQueue(database, attr);
    TestBulk();
    delete queue;
    queue = 0;
    //queue = shared_ptr<QueueBase>(new SimpleQueue(database, attr));
    //TestDirect(queue.get());
    attr.SetNumChannels(10);
	//DEBUG("%s : Size %u, MaxThresh %u, Chans %u\n",__PRETTY_FUNCTION__, attr.GetLength(), attr.GetMaxThreshold(), attr.GetNumChannels());
    queue = new SimpleQueue(database, attr);
    TestBulk();
    delete queue;
    queue = 0;
    queue = new SimpleQueue(database, attr);
    TestDirect();
    delete queue;
    queue = 0;
    queue = new SimpleQueue(database, attr);
    CommunicationTest();
    delete queue;
    queue = 0;
    queue = new SimpleQueue(database, attr);
    DequeueBlockTest();
    delete queue;
    queue = 0;
    queue = new SimpleQueue(database, attr);
    MaxThreshGrowTest();
    delete queue;
    queue = 0;
    queue = new SimpleQueue(database, attr);
    GrowTest();
}

void QueueTest::ThresholdQueueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    shared_ptr<Database> database = shared_ptr<Database>(new MockDatabase);
    SimpleQueueAttr attr;
    attr.SetLength(30).SetMaxThreshold(10).SetNumChannels(1);
	//DEBUG("%s : Size %u, MaxThresh %u, Chans %u\n",__PRETTY_FUNCTION__, attr.GetLength(), attr.GetMaxThreshold(), attr.GetNumChannels());
    queue = new ThresholdQueue(database, attr);
    TestBulk();
    delete queue;
    queue = 0;
    //queue = shared_ptr<QueueBase>(new ThresholdQueue(database, attr));
    //TestDirect(queue.get());
    attr.SetNumChannels(10);
	//DEBUG("%s : Size %u, MaxThresh %u, Chans %u\n",__PRETTY_FUNCTION__, attr.GetLength(), attr.GetMaxThreshold(), attr.GetNumChannels());
    queue = new ThresholdQueue(database, attr);
    CommunicationTest();
    delete queue;
    queue = 0;
    queue = new ThresholdQueue(database, attr);
    MaxThreshGrowTest();
    delete queue;
    queue = 0;
    queue = new ThresholdQueue(database, attr);
    GrowTest();
}

void QueueTest::TestBulk() {
    unsigned maxthresh = queue->MaxThreshold();
    unsigned channels = queue->NumChannels();
    //printf("Size %u, Thresh %u, Chans %u\n", queue->QueueLength(), maxthresh, channels);
    std::vector< std::deque<char> > data(channels);
    CPPUNIT_ASSERT(queue->Empty());
    // fill it up with deterministic garbage
    srand(1);
    unsigned indexbase = 0;
    while (true) {
        int amount = queue->Freespace() < maxthresh ? queue->Freespace() : maxthresh;
        if (amount == 0) break;
        for (unsigned chan = 0; chan < channels; ++chan) {
            unsigned index = indexbase;
            char *ptr = static_cast<char*>(queue->GetRawEnqueuePtr(amount, chan));
            char *end = ptr + amount;
            while (ptr != end) {
                *ptr = (char)rand();
                //DEBUG("(%02u, %04u) = %d\n", chan, index, *ptr);
                data[chan].push_back(*ptr);
                ++index;
                ++ptr;
            }
        }
        queue->Enqueue(amount);
        indexbase += amount;
    }
    queue->ShutdownWriter();
    CPPUNIT_ASSERT(queue->Full());
    srand(1);
    indexbase = 0;
    while (true) {
        int amount = queue->Count() < maxthresh ? queue->Count() : maxthresh;
        if (amount == 0) break;
        for (unsigned chan = 0; chan < channels; ++chan) {
            unsigned index = indexbase;
            const char *ptr = static_cast<const char*>(queue->GetRawDequeuePtr(amount, chan));
            const char *end = ptr + amount;
            while (ptr != end) {
                char val = data[chan].front();
                data[chan].pop_front();
                //DEBUG("(%02u, %04u) = %d =? %d\n", chan, index, *ptr, val);
                ++index;
                CPPUNIT_ASSERT(*ptr == val);
                ++ptr;
            }
        }
        queue->Dequeue(amount);
        indexbase += amount;
    }
    CPPUNIT_ASSERT(queue->Empty());
}

void QueueTest::TestDirect() {
    unsigned maxthresh = queue->MaxThreshold();
    unsigned channels = queue->NumChannels();
    unsigned qsize = queue->QueueLength();
    std::vector< char > data(channels * qsize);
    std::vector< std::deque<char> > dataqueued(channels);
    CPPUNIT_ASSERT(queue->Empty());
    // fill it up with deterministic garbage
    srand(1);
    for (unsigned i = 0; i < 2; ++i) {
        for (unsigned amount = 1; amount <= maxthresh; amount += maxthresh/10 + 1) {
            for (unsigned chan = 0; chan < channels; ++chan) {
                for ( unsigned j = 0; j < amount; ++j) {
                    char val = (char)rand();
                    data[(qsize * chan) + j] = val;
                    dataqueued[chan].push_back(val);
                }
            }
            if (channels == 1) {
                queue->RawEnqueue(&data[0], amount);
            } else {
                queue->RawEnqueue(&data[0], amount, channels, qsize);
            }
            if (channels == 1) {
                CPPUNIT_ASSERT(queue->RawDequeue(&data[0], amount));
            } else {
                CPPUNIT_ASSERT(queue->RawDequeue(&data[0], amount, channels, qsize));
            }
            for (unsigned chan = 0; chan < channels; ++chan) {
                for ( unsigned j = 0; j < amount; ++j) {
                    char val = data[(qsize * chan) + j];
                    CPPUNIT_ASSERT(dataqueued[chan].front() == val);
                    dataqueued[chan].pop_front();
                }
            }
        }
    }
    CPPUNIT_ASSERT(queue->Empty());
}

// Tests normal communication and blocking on the enqueue side
void QueueTest::CommunicationTest() {
    Reset();
    std::auto_ptr<Pthread> enqueuer = std::auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &QueueTest::EnqueueData));
    std::auto_ptr<Pthread> dequeuer = std::auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &QueueTest::DequeueData));
    enqueuer->Start();
    while (queue->WriteRequest() == 0);
    dequeuer->Start();
    {
        PthreadMutexProtected al(dequeue_lock);
        while (dequeue_num == 0) {
            dequeue_cond.Wait(dequeue_lock);
        }
    }
    {
        PthreadMutexProtected al(enqueue_lock);
        enqueue_stop = true;
    }
    enqueuer->Join();
    dequeuer->Join();
    CPPUNIT_ASSERT(dequeue_num == enqueue_num);
    CPPUNIT_ASSERT(!enqueue_fail);
    CPPUNIT_ASSERT(!dequeue_fail);
}

// Test that dequeue will block and then test that enqueue properly
// dies when the dequeue side shuts down.
void QueueTest::DequeueBlockTest() {
    Reset();
    std::auto_ptr<Pthread> enqueuer = std::auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &QueueTest::EnqueueData));
    std::auto_ptr<Pthread> dequeuer = std::auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &QueueTest::DequeueData));
    dequeuer->Start();
    while (queue->ReadRequest() == 0);
    enqueuer->Start();
    {
        PthreadMutexProtected al(dequeue_lock);
        dequeue_stop = true;
    }
    enqueuer->Join();
    dequeuer->Join();
    CPPUNIT_ASSERT(!dequeue_fail);
    CPPUNIT_ASSERT(enqueue_fail);
}

void QueueTest::MaxThreshGrowTest() {
    unsigned maxthresh = queue->MaxThreshold();
    unsigned numchan = queue->NumChannels();
    char *ptr = 0;
    const char *cptr = 0;
    std::vector<char> buff(4*maxthresh * numchan);
    srand(0);
    for (unsigned i = 0; i < buff.size(); ++i) {
        buff[i] = (char)rand();
    }
    maxthresh *= 2;

    for (unsigned i = 0; i < numchan; ++i) {
        ptr = (char*)queue->GetRawEnqueuePtr(maxthresh, i);
        CPPUNIT_ASSERT(ptr);
        memcpy(ptr, &buff[2*maxthresh*i], maxthresh);
    }
    CPPUNIT_ASSERT(queue->MaxThreshold() >= maxthresh);
    queue->Enqueue(maxthresh);

    // Test that growth works
    unsigned len = queue->QueueLength() * 2;
    queue->Grow(len, maxthresh);
    CPPUNIT_ASSERT(queue->QueueLength() >= len);
    CPPUNIT_ASSERT(queue->MaxThreshold() >= maxthresh);
    for (unsigned i = 0; i < numchan; ++i) {
        ptr = (char*)queue->GetRawEnqueuePtr(maxthresh, i);
        CPPUNIT_ASSERT(ptr);
        memcpy(ptr, &buff[2*maxthresh*i + maxthresh], maxthresh);
    }
    queue->Enqueue(maxthresh);

    maxthresh *= 2;
    for (unsigned i = 0; i < numchan; ++i) {
        cptr = (const char*)queue->GetRawDequeuePtr(maxthresh, i);
        CPPUNIT_ASSERT(cptr);
        CPPUNIT_ASSERT(memcmp(cptr, &buff[maxthresh * i], maxthresh) == 0);
    }
    CPPUNIT_ASSERT(queue->MaxThreshold() >= maxthresh);
}

void QueueTest::GrowTest() {
    unsigned maxthresh = queue->MaxThreshold();
    unsigned numchan = queue->NumChannels();
    char *ptr = 0;
    const char *cptr = 0;
    std::vector<char> buff(4*maxthresh * numchan);
    srand(0);
    for (unsigned i = 0; i < buff.size(); ++i) {
        buff[i] = (char)rand();
    }
    maxthresh *= 2;

    for (unsigned i = 0; i < numchan; ++i) {
        ptr = (char*)queue->GetRawEnqueuePtr(maxthresh, i);
        CPPUNIT_ASSERT(ptr);
        memcpy(ptr, &buff[2*maxthresh*i], maxthresh);
    }
    CPPUNIT_ASSERT(queue->MaxThreshold() >= maxthresh);
    queue->Enqueue(maxthresh);

    cptr = (const char*)queue->GetRawDequeuePtr(maxthresh, 0);
    unsigned len = queue->QueueLength() * 2;
    queue->Grow(len, maxthresh);
    CPPUNIT_ASSERT(queue->QueueLength() >= len);
    CPPUNIT_ASSERT(queue->MaxThreshold() >= maxthresh);
    for (unsigned i = 0; i < numchan; ++i) {
        ptr = (char*)queue->GetRawEnqueuePtr(maxthresh, i);
        CPPUNIT_ASSERT(ptr);
        memcpy(ptr, &buff[2*maxthresh*i + maxthresh], maxthresh);
    }
    queue->Enqueue(maxthresh);
    queue->Dequeue(0);

    ptr = (char*)queue->GetRawEnqueuePtr(1, 0);
    maxthresh *= 2;
    for (unsigned i = 0; i < numchan; ++i) {
        cptr = (const char*)queue->GetRawDequeuePtr(maxthresh, i);
        CPPUNIT_ASSERT(cptr);
        CPPUNIT_ASSERT(memcmp(cptr, &buff[maxthresh * i], maxthresh) == 0);
    }
    CPPUNIT_ASSERT(queue->MaxThreshold() >= maxthresh);
    queue->Enqueue(0);
    queue->Dequeue(maxthresh);
    CPPUNIT_ASSERT(queue->Count() == 0);
}

