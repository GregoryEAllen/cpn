
#include "D4RTestQueue.h"
#include "ThrowingAssert.h"
#include <stdio.h>

namespace D4R {
    TestQueue::TestQueue(unsigned initialsize, const std::string &name_)
        : detected(false), 
        aborted(false),
        queuesize(initialsize), 
        count(0),
        enqueue_amount(0),
        dequeue_amount(0),
        name(name_)
    {
        Logger::Name(name);
    }

    TestQueue::TestQueue(const Variant &queued)
        : detected(false),
        aborted(false),
        queuesize(queued["size"].AsUnsigned()),
        count(0),
        enqueue_amount(0),
        dequeue_amount(0),
        name(queued["name"].AsString())
    {
        Logger::Name(name);
    }

    void TestQueue::Enqueue(unsigned amount) {
        PthreadMutexProtected al(lock);
        Trace("Enqueue %u", amount);
        enqueue_amount = amount;
        while (WriteBlocked()) {
            Trace("Blocking");
            WriteBlock(queuesize);
        }
        count += amount;
        enqueue_amount = 0;
        Signal();
    }

    void TestQueue::Dequeue(unsigned amount) {
        PthreadMutexProtected al(lock);
        Trace("Dequeue %u", amount);
        dequeue_amount = amount;
        while (ReadBlocked()) {
            Trace("Blocking");
            ReadBlock();
        }
        count -= amount;
        dequeue_amount = 0;
        Signal();
    }

    void TestQueue::Abort() {
        PthreadMutexProtected al(lock);
        Trace("Aborting");
        aborted = true;
        Signal();
    }

    bool TestQueue::WriteBlocked() {
        if (aborted) { throw TestQueueAbortException(name); }
        return queuesize - count < enqueue_amount;
    }

    bool TestQueue::ReadBlocked() {
        if (aborted) { throw TestQueueAbortException(name); }
        return count < dequeue_amount;
    }

    bool TestQueue::Detected() {
        PthreadMutexProtected al(lock);
        bool d = detected;
        detected = false;
        return d;
    }

    void TestQueue::Detect() {
        detected = true;
        queuesize = count + enqueue_amount;
        Debug("increased queue size", name.c_str());
        Signal();
    }
}
