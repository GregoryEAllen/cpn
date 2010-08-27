
#include "RemoteQueueTest.h"
#include <cppunit/TestAssert.h>
#include "QueueAttr.h"
#include "ConnectionServer.h"
#include "Database.h"
#include "RemoteQueue.h"
#include "PthreadFunctional.h"
#include "ErrnoException.h"
#include <stdlib.h>
CPPUNIT_TEST_SUITE_REGISTRATION( RemoteQueueTest );

using CPN::shared_ptr;
using CPN::Database;
using CPN::SimpleQueueAttr;
using CPN::RemoteQueue;
using CPN::QueueBase;
using CPN::auto_ptr;
using CPN::ConnectionServer;

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, ## __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

const unsigned QUEUESIZE = 5;
const char data[] = { 'a', 'b', 'c', 'd', 'e' };


void RemoteQueueTest::setUp() {

    fail = false;
    stopenqueue = false;
    stopdequeue = false;
    numenqueued = 0;
    numdequeued = 0;
    enqueuedead = false;
    dequeuedead = false;


    database = CPN::Database::Local();

    database->LogLevel(Logger::WARNING);
    // We don't have any nodes so we must have D4R off.
    database->UseD4R(false);


    SockAddrList addrs = SocketAddress::CreateIP("0.0.0.0", "");
    server = shared_ptr<ConnectionServer>(new ConnectionServer(addrs, database));
    server->Disable();
    servert = shared_ptr<Pthread>(CreatePthreadFunctional(this, &RemoteQueueTest::PollServer));
    CPPUNIT_ASSERT_EQUAL(0, servert->Error());
    servert->Start();


    SocketAddress addr = server->GetAddress();
    hostkey = database->SetupHost("bogus", addr.GetHostName(), addr.GetServName(), this);

    nodekey = database->CreateNodeKey(hostkey, "Bogus");
 
    writerkey = database->GetCreateWriterKey(nodekey, "writer");
    readerkey = database->GetCreateReaderKey(nodekey, "reader");
    database->ConnectEndpoints(writerkey, readerkey);

    SimpleQueueAttr attr;
    attr.SetLength(QUEUESIZE*2).SetMaxThreshold(QUEUESIZE).SetNumChannels(1);
    attr.SetReaderKey(readerkey).SetWriterKey(writerkey);
    attr.SetReaderNodeKey(nodekey).SetWriterNodeKey(nodekey);
    attr.SetAlpha(0.5);

    rendp = shared_ptr<RemoteQueue>(
            new RemoteQueue(
                database,
                RemoteQueue::READ,
                server.get(),
                &remotequeueholder,
                attr
                )
            );
    remotequeueholder.AddQueue(rendp);
    rendp->Start();
    wendp = shared_ptr<RemoteQueue>(
            new RemoteQueue(
                database,
                RemoteQueue::WRITE,
                server.get(),
                &remotequeueholder,
                attr
                )
            );
    remotequeueholder.AddQueue(wendp);
    wendp->Start();

    wqueue = wendp;
    rqueue = rendp;
}

void RemoteQueueTest::tearDown() {
    database->Terminate();
    server->Wakeup();
    servert.reset();
    server->Close();
    remotequeueholder.Shutdown();

    rendp.reset();
    wendp.reset();
    wqueue.reset();
    rqueue.reset();
    server.reset();
    database.reset();
}

void RemoteQueueTest::CommunicationTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    // Send data across and wait for it at the other end.
    // Then verify that reading more blocks.
    // then verify that filling up the queue and then some will block the writer.
    server->Enable();
    auto_ptr<Pthread> enqueuer = auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &RemoteQueueTest::EnqueueData));
    CPPUNIT_ASSERT_EQUAL(0, enqueuer->Error());
    auto_ptr<Pthread> dequeuer = auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &RemoteQueueTest::DequeueData));
    CPPUNIT_ASSERT_EQUAL(0, dequeuer->Error());

    dequeuer->Start();
    while (wqueue->ReadRequest() == 0) {
        Pthread::Yield();
    }
    CPPUNIT_ASSERT(wqueue->ReadRequest() == sizeof(data));
    enqueuer->Start();

    {
        PthreadMutexProtected al(lock);
        while (true) {
            CPPUNIT_ASSERT(!fail);
            if (numdequeued > sizeof(data)) {
                break;
            }
            cond.Wait(lock);
        }

        stopdequeue = true;

        while (true) {
            if (dequeuedead) { break; }
            cond.Wait(lock);
        }
    }
    dequeuer->Join();

    rqueue->ShutdownReader();
    while (wqueue->IsReaderShutdown() == false) {
        Pthread::Yield();
    }
    enqueuer->Join();
    CPPUNIT_ASSERT(!fail);
}

void *RemoteQueueTest::EnqueueData() {
    try {
        while (true) {
            wqueue->RawEnqueue(data, sizeof(data));
            {
                PthreadMutexProtected al(lock);
                numenqueued += sizeof(data);
                cond.Signal();
                if (stopenqueue) {
                    break;
                }
            }
        }
    } catch (const std::exception &e) {
        DEBUG("Exception on enqueue: %s\n", e.what());
    }
    PthreadMutexProtected al(lock);
    enqueuedead = true;
    cond.Signal();
    return 0;
}

void *RemoteQueueTest::DequeueData() {
    try {
        while (true) {
            const void *ptr = rqueue->GetRawDequeuePtr(sizeof(data), 0);
            if (ptr) {
                PthreadMutexProtected al(lock);
                cond.Signal();
                if (memcmp(ptr, data, sizeof(data)) != 0) {
                    fail = true;
                    break;
                }
                numdequeued += sizeof(data);
                rqueue->Dequeue(sizeof(data));
                if (stopdequeue) {
                    break;
                }
            } else {
                break;
            }
        }
    } catch (const std::exception &e) {
        DEBUG("Exception on dequeue: %s\n", e.what());
    }
    PthreadMutexProtected al(lock);
    dequeuedead = true;
    cond.Signal();
    return 0;
}

// Test what happens when we write a bunch of stuff then
// shutdown
void RemoteQueueTest::EndOfWriteQueueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);

    server->Enable();
    auto_ptr<Pthread> enqueuer = auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &RemoteQueueTest::EnqueueData));
    CPPUNIT_ASSERT_EQUAL(0, enqueuer->Error());
    auto_ptr<Pthread> dequeuer = auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &RemoteQueueTest::DequeueData));
    CPPUNIT_ASSERT_EQUAL(0, dequeuer->Error());
    enqueuer->Start();
    while (rqueue->WriteRequest() == 0) {
        Pthread::Yield();
    }
    wqueue->ShutdownWriter();
    enqueuer->Join();
    dequeuer->Start();
    {
        PthreadMutexProtected al(lock);
        while (true) {
            if (dequeuedead) {
                break;
            }
            cond.Wait(lock);
        }
    }
    dequeuer->Join();
    CPPUNIT_ASSERT(rqueue->IsWriterShutdown());
    CPPUNIT_ASSERT(!fail);
    CPPUNIT_ASSERT(numdequeued == numenqueued);
}

void RemoteQueueTest::EndOfReadQueueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);

    server->Enable();
    rqueue->ShutdownReader();
    while (wqueue->IsReaderShutdown() == false) {
        Pthread::Yield();
    }
    CPPUNIT_ASSERT(rqueue->IsReaderShutdown());
    CPPUNIT_ASSERT(wqueue->IsReaderShutdown());
}

// Test that the connection aborts correctly for end of read
// with data in the queue
void RemoteQueueTest::EndOfReadQueueTest2() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);

    server->Enable();
    auto_ptr<Pthread> enqueuer = auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &RemoteQueueTest::EnqueueData));
    CPPUNIT_ASSERT_EQUAL(0, enqueuer->Error());
    enqueuer->Start();
    while (rqueue->WriteRequest() == 0) {
        Pthread::Yield();
    }
    rqueue->ShutdownReader();
    while (wqueue->IsReaderShutdown() == false) {
        Pthread::Yield();
    }
    enqueuer->Join();
    CPPUNIT_ASSERT(rqueue->IsReaderShutdown());
    CPPUNIT_ASSERT(wqueue->IsReaderShutdown());
}

void RemoteQueueTest::WriteBlockWithNoFDTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    auto_ptr<Pthread> enqueuer = auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &RemoteQueueTest::EnqueueData));
    CPPUNIT_ASSERT_EQUAL(0, enqueuer->Error());
    enqueuer->Start();
    // Wait for the enqueuer to block
    while (wqueue->WriteRequest() == 0) {
        Pthread::Yield();
    }

    server->Enable();

    // Now wait for the reader side to recieve the block
    while (rqueue->WriteRequest() == 0) {
        Pthread::Yield();
    }
    wqueue->ShutdownWriter();
    {
        PthreadMutexProtected al(lock);
        while (true) {
            if (enqueuedead) { break; }
            cond.Wait(lock);
        }
    }
    CPPUNIT_ASSERT(!rqueue->IsWriterShutdown());
    CPPUNIT_ASSERT(wqueue->IsWriterShutdown());
    CPPUNIT_ASSERT(rqueue->Full());
    CPPUNIT_ASSERT(wqueue->Full());
    rqueue->ShutdownReader();
}

void RemoteQueueTest::WriteEndWithNoFDTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    auto_ptr<Pthread> enqueuer = auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &RemoteQueueTest::EnqueueData));
    CPPUNIT_ASSERT_EQUAL(0, enqueuer->Error());
    enqueuer->Start();
    // Wait for the enqueuer to block
    while (wqueue->WriteRequest() == 0) {
        Pthread::Yield();
    }

    wqueue->ShutdownWriter();
    {
        PthreadMutexProtected al(lock);
        while (true) {
            if (enqueuedead) { break; }
            cond.Wait(lock);
        }
    }
    enqueuer->Join();

    server->Enable();

    std::auto_ptr<Pthread> dequeuer = std::auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &RemoteQueueTest::DequeueData));
    CPPUNIT_ASSERT_EQUAL(0, dequeuer->Error());
    dequeuer->Start();
    {
        PthreadMutexProtected al(lock);
        while (true) {
            if (dequeuedead) { break; }
            cond.Wait(lock);
        }
    }
    dequeuer->Join();
    CPPUNIT_ASSERT(rqueue->IsWriterShutdown());
    CPPUNIT_ASSERT(wqueue->IsWriterShutdown());
    CPPUNIT_ASSERT(numenqueued == numdequeued);
}

void RemoteQueueTest::MaxThreshGrowTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    server->Enable();
    unsigned maxthresh = wqueue->MaxThreshold();
    unsigned numchan = wqueue->NumChannels();
    char *ptr = 0;
    const char *cptr = 0;
    std::vector<char> buff(4*maxthresh * numchan);
    srand(0);
    for (unsigned i = 0; i < buff.size(); ++i) {
        buff[i] = (char)rand();
    }
    maxthresh *= 2;

    for (unsigned i = 0; i < numchan; ++i) {
        ptr = (char*)wqueue->GetRawEnqueuePtr(maxthresh, i);
        CPPUNIT_ASSERT(ptr);
        memcpy(ptr, &buff[2*maxthresh*i], maxthresh);
    }
    CPPUNIT_ASSERT(wqueue->MaxThreshold() >= maxthresh);
    wqueue->Enqueue(maxthresh);
    while (rqueue->MaxThreshold() != wqueue->MaxThreshold()) {
        Pthread::Yield();
    }

    // Test that growth works
    unsigned len = rqueue->QueueLength() * 2;
    rqueue->Grow(len, maxthresh);
    while (rqueue->QueueLength() != wqueue->QueueLength()) {
        Pthread::Yield();
    }
    CPPUNIT_ASSERT(wqueue->QueueLength() >= len);
    CPPUNIT_ASSERT(wqueue->MaxThreshold() >= maxthresh);
    for (unsigned i = 0; i < numchan; ++i) {
        ptr = (char*)wqueue->GetRawEnqueuePtr(maxthresh, i);
        CPPUNIT_ASSERT(ptr);
        memcpy(ptr, &buff[2*maxthresh*i + maxthresh], maxthresh);
    }
    wqueue->Enqueue(maxthresh);


    maxthresh *= 2;
    while (rqueue->Count() < maxthresh) {
        Pthread::Yield();
    }
    for (unsigned i = 0; i < numchan; ++i) {
        cptr = (const char*)rqueue->GetRawDequeuePtr(maxthresh, i);
        CPPUNIT_ASSERT(cptr);
        CPPUNIT_ASSERT(memcmp(cptr, &buff[maxthresh * i], maxthresh) == 0);
    }
    while (rqueue->MaxThreshold() != wqueue->MaxThreshold()) {
        Pthread::Yield();
    }
    CPPUNIT_ASSERT(wqueue->MaxThreshold() >= maxthresh);
    wqueue->ShutdownWriter();
    rqueue->ShutdownReader();
}

void RemoteQueueTest::GrowTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    server->Enable();
    unsigned maxthresh = wqueue->MaxThreshold();
    unsigned numchan = wqueue->NumChannels();
    char *ptr = 0;
    const char *cptr = 0;
    std::vector<char> buff(4*maxthresh * numchan);
    srand(0);
    for (unsigned i = 0; i < buff.size(); ++i) {
        buff[i] = (char)rand();
    }
    maxthresh *= 2;

    for (unsigned i = 0; i < numchan; ++i) {
        ptr = (char*)wqueue->GetRawEnqueuePtr(maxthresh, i);
        CPPUNIT_ASSERT(ptr);
        memcpy(ptr, &buff[2*maxthresh*i], maxthresh);
    }
    CPPUNIT_ASSERT(wqueue->MaxThreshold() >= maxthresh);
    wqueue->Enqueue(maxthresh);
    while (rqueue->MaxThreshold() != wqueue->MaxThreshold()) {
        Pthread::Yield();
    }

    cptr = (const char*)rqueue->GetRawDequeuePtr(maxthresh, 0);
    unsigned len = wqueue->QueueLength() * 2;
    wqueue->Grow(len, maxthresh);
    while (rqueue->QueueLength() != wqueue->QueueLength()) {
        Pthread::Yield();
    }
    CPPUNIT_ASSERT(rqueue->QueueLength() >= len);
    CPPUNIT_ASSERT(rqueue->MaxThreshold() >= maxthresh);
    for (unsigned i = 0; i < numchan; ++i) {
        ptr = (char*)wqueue->GetRawEnqueuePtr(maxthresh, i);
        CPPUNIT_ASSERT(ptr);
        memcpy(ptr, &buff[2*maxthresh*i + maxthresh], maxthresh);
    }
    wqueue->Enqueue(maxthresh);
    rqueue->Dequeue(0);

    ptr = (char*)wqueue->GetRawEnqueuePtr(1, 0);
    maxthresh *= 2;
    while (rqueue->Count() < maxthresh) {
        Pthread::Yield();
    }
    for (unsigned i = 0; i < numchan; ++i) {
        cptr = (const char*)rqueue->GetRawDequeuePtr(maxthresh, i);
        CPPUNIT_ASSERT(cptr);
        CPPUNIT_ASSERT(memcmp(cptr, &buff[maxthresh * i], maxthresh) == 0);
    }
    while (rqueue->MaxThreshold() != wqueue->MaxThreshold()) {
        Pthread::Yield();
    }
    CPPUNIT_ASSERT(wqueue->MaxThreshold() >= maxthresh);
    wqueue->Enqueue(0);
    rqueue->Dequeue(maxthresh);
    CPPUNIT_ASSERT(rqueue->Count() == 0);
    wqueue->ShutdownWriter();
    rqueue->ShutdownReader();
}



void RemoteQueueTest::NotifyTerminate() {
    wqueue->NotifyTerminate();
    rqueue->NotifyTerminate();
    server->Wakeup();
}

void *RemoteQueueTest::PollServer() {
    try {
        while (!database->IsTerminated()) {
            server->Poll();
        }
    } catch (const ErrnoException &e) {
        // server was closed...
    }
    return 0;
}
