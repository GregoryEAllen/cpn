
#include "QueueTest.h"
#include "QueueBase.h"
#include "ThresholdQueue.h"
#include "CPNSimpleQueue.h"
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

void QueueTest::setUp() {
}

void QueueTest::tearDown() {
}

void QueueTest::SimpleQueueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    unsigned size = 309;
    unsigned maxthresh = 10;
    unsigned chans = 1;
	//DEBUG("%s : Size %u, MaxThresh %u, Chans %u\n",__PRETTY_FUNCTION__, size, maxthresh, chans);
    shared_ptr<QueueBase> queue = shared_ptr<QueueBase>(new CPN::SimpleQueue(size, maxthresh, chans));
    TestBulk(queue.get());
    TestDirect(queue.get());
    TestBulk(queue.get());
    TestDirect(queue.get());
    chans = 10;
	//DEBUG("%s : Size %u, MaxThresh %u, Chans %u\n",__PRETTY_FUNCTION__, size, maxthresh, chans);
    queue = shared_ptr<QueueBase>(new CPN::SimpleQueue(size, maxthresh, chans));
    TestBulk(queue.get());
    TestDirect(queue.get());
    TestBulk(queue.get());
    TestDirect(queue.get());
}

void QueueTest::ThresholdQueueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    unsigned size = 30;
    unsigned maxthresh = 10;
    unsigned chans = 1;
	//DEBUG("%s : Size %u, MaxThresh %u, Chans %u\n",__PRETTY_FUNCTION__, size, maxthresh, chans);
    shared_ptr<QueueBase> queue = shared_ptr<QueueBase>(new ThresholdQueue(size, maxthresh, chans));
    TestBulk(queue.get());
    TestDirect(queue.get());
    chans = 2;
	//DEBUG("%s : Size %u, MaxThresh %u, Chans %u\n",__PRETTY_FUNCTION__, size, maxthresh, chans);
    queue = shared_ptr<QueueBase>(new ThresholdQueue(size, maxthresh, chans));
    TestBulk(queue.get());
    TestDirect(queue.get());

}

void QueueTest::TestBulk(QueueBase *queue) {
    unsigned maxthresh = queue->MaxThreshold();
    unsigned channels = queue->NumChannels();
    //printf("Size %u, Thresh %u, Chans %u\n", queue->QueueLength(), maxthresh, channels);
    std::vector< std::deque<char> > data(channels);
    CPPUNIT_ASSERT(0 == queue->GetRawEnqueuePtr(maxthresh + 1));
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
    CPPUNIT_ASSERT(queue->Full());
    CPPUNIT_ASSERT(0 == queue->GetRawEnqueuePtr(1));
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

void QueueTest::TestDirect(QueueBase *queue) {
    unsigned maxthresh = queue->MaxThreshold();
    unsigned channels = queue->NumChannels();
    unsigned qsize = queue->QueueLength();
    std::vector< char > data(channels * qsize);
    std::vector< std::deque<char> > dataqueued(channels);
    CPPUNIT_ASSERT(0 == queue->GetRawEnqueuePtr(maxthresh + 1));
    CPPUNIT_ASSERT(queue->Empty());
    // fill it up with deterministic garbage
    srand(1);
    for (unsigned i = 0; i < (3*qsize/maxthresh); ++i) {
        for (unsigned amount = 1; amount <= maxthresh; amount += maxthresh/10 + 1) {
            for (unsigned chan = 0; chan < channels; ++chan) {
                for ( unsigned j = 0; j < amount; ++j) {
                    char val = (char)rand();
                    data[(qsize * chan) + j] = val;
                    dataqueued[chan].push_back(val);
                }
            }
            if (channels == 1) {
                CPPUNIT_ASSERT(queue->RawEnqueue(&data[0], amount));
            } else {
                CPPUNIT_ASSERT(queue->RawEnqueue(&data[0], amount, channels, qsize));
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

