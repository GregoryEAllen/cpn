

#include "StreamEndpointTest.h"
#include <cppunit/TestAssert.h>

#include "QueueAttr.h"
#include "Database.h"

#include "PthreadFunctional.h"

#include <tr1/memory>
#include <vector>
#include <algorithm>

CPPUNIT_TEST_SUITE_REGISTRATION( StreamEndpointTest );

using std::tr1::shared_ptr;
using std::tr1::dynamic_pointer_cast;
using namespace CPN;

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, ## __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

const unsigned QUEUESIZE = 5;
const char data[] = { 'a', 'b', 'c', 'd', 'e' };
const Key_t RKEY = 1;
const Key_t WKEY = 2;


void StreamEndpointTest::setUp() {

    fail = false;
    stopenqueue = false;
    stopdequeue = false;
    numenqueued = 0;
    numdequeued = 0;
    enqueuedead = false;
    dequeuedead = false;


    database = CPN::Database::Local();

    database->LogLevel(Logger::WARNING);

    rfd = shared_ptr<FileFuture>(new FileFuture);
    wfd = shared_ptr<FileFuture>(new FileFuture);

    SimpleQueueAttr attr;
    attr.SetLength(QUEUESIZE*2).SetMaxThreshold(QUEUESIZE).SetNumChannels(1);
    attr.SetReaderKey(RKEY).SetWriterKey(WKEY);

    wendp = shared_ptr<SocketEndpoint>(new SocketEndpoint(
        database,
        SocketEndpoint::WRITE,
        this, attr
        ));

    wqueue = wendp;

    rendp = shared_ptr<SocketEndpoint>(new SocketEndpoint(
                database,
                SocketEndpoint::READ,
                this,
                attr));

    rqueue = rendp;
}

void StreamEndpointTest::tearDown() {
    wqueue.reset();
    rqueue.reset();
    wendp.reset();
    rendp.reset();
    database.reset();
}

void StreamEndpointTest::CommunicationTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    // Send data across and wait for it at the other end.
    // Then verify that reading more blocks.
    // then verify that filling up the queue and then some will block the writer.
    SetupDescriptors();
    std::auto_ptr<Pthread> enqueuer = std::auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &StreamEndpointTest::EnqueueData));
    std::auto_ptr<Pthread> dequeuer = std::auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &StreamEndpointTest::DequeueData));

    dequeuer->Start();
    while (wqueue->ReadRequest() == 0) {
        Poll(0);
    }
    CPPUNIT_ASSERT(wqueue->ReadRequest() == sizeof(data));
    enqueuer->Start();

    while (true) {
        {
            PthreadMutexProtected al(lock);
            CPPUNIT_ASSERT(!fail);
            if (numdequeued > sizeof(data)) {
                break;
            }
        }
        Poll(0);
    }

    {
        PthreadMutexProtected al(lock);
        stopdequeue = true;
    }
    while (true) {
        Poll(0);
        {
            PthreadMutexProtected al(lock);
            if (dequeuedead) { break; }
        }
    }
    dequeuer->Join();

    rqueue->ShutdownReader();
    while (wqueue->IsReaderShutdown() == false) {
        Poll(0);
    }
    enqueuer->Join();
    CPPUNIT_ASSERT(!fail);
}

void *StreamEndpointTest::EnqueueData() {
    try {
        while (true) {
            wqueue->RawEnqueue(data, sizeof(data));
            {
                PthreadMutexProtected al(lock);
                numenqueued += sizeof(data);
                if (stopenqueue) {
                    break;
                }
            }
        }
    } catch (...) {
    }
    PthreadMutexProtected al(lock);
    enqueuedead = true;
    return 0;
}

void *StreamEndpointTest::DequeueData() {
    try {
        while (true) {
            const void *ptr = rqueue->GetRawDequeuePtr(sizeof(data), 0);
            if (ptr) {
                PthreadMutexProtected al(lock);
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
    } catch (...) {
    }
    PthreadMutexProtected al(lock);
    dequeuedead = true;
    return 0;
}

// Test what happens when we write a bunch of stuff then
// shutdown
void StreamEndpointTest::EndOfWriteQueueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    SetupDescriptors();

    std::auto_ptr<Pthread> enqueuer = std::auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &StreamEndpointTest::EnqueueData));
    std::auto_ptr<Pthread> dequeuer = std::auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &StreamEndpointTest::DequeueData));
    enqueuer->Start();
    while (rqueue->WriteRequest() == 0) {
        Poll(0);
    }
    wqueue->ShutdownWriter();
    enqueuer->Join();
    dequeuer->Start();
    while (true) {
        Poll(0);
        {
            PthreadMutexProtected al(lock);
            if (dequeuedead) {
                break;
            }
        }
    }
    dequeuer->Join();
    CPPUNIT_ASSERT(rqueue->IsWriterShutdown());
    CPPUNIT_ASSERT(!fail);
    CPPUNIT_ASSERT(numdequeued == numenqueued);
}

void StreamEndpointTest::EndOfReadQueueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    SetupDescriptors();

    rqueue->ShutdownReader();
    while (wqueue->IsReaderShutdown() == false) {
        Poll(0);
    }
    CPPUNIT_ASSERT(rqueue->IsReaderShutdown());
    CPPUNIT_ASSERT(wqueue->IsReaderShutdown());
}

// Test that the connection aborts correctly for end of read
// with data in the queue
void StreamEndpointTest::EndOfReadQueueTest2() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    SetupDescriptors();

    std::auto_ptr<Pthread> enqueuer = std::auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &StreamEndpointTest::EnqueueData));
    enqueuer->Start();
    while (rqueue->WriteRequest() == 0) {
        Poll(0);
    }
    rqueue->ShutdownReader();
    while (wqueue->IsReaderShutdown() == false) {
        Poll(0);
    }
    enqueuer->Join();
    CPPUNIT_ASSERT(rqueue->IsReaderShutdown());
    CPPUNIT_ASSERT(wqueue->IsReaderShutdown());
}

void StreamEndpointTest::WriteBlockWithNoFDTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    std::auto_ptr<Pthread> enqueuer = std::auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &StreamEndpointTest::EnqueueData));
    enqueuer->Start();
    // Wait for the enqueuer to block
    while (wqueue->WriteRequest() == 0) {
        Poll(0);
    }

    SetupDescriptors();

    // Now wait for the reader side to recieve the block
    while (rqueue->WriteRequest() == 0) {
        Poll(0);
    }
    wqueue->ShutdownWriter();
    while (true) {
        Poll(0);
        {
            PthreadMutexProtected al(lock);
            if (enqueuedead) { break; }
        }
    }
    CPPUNIT_ASSERT(!rqueue->IsWriterShutdown());
    CPPUNIT_ASSERT(wqueue->IsWriterShutdown());
    CPPUNIT_ASSERT(rqueue->Full());
    CPPUNIT_ASSERT(wqueue->Full());
}

void StreamEndpointTest::WriteEndWithNoFDTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    std::auto_ptr<Pthread> enqueuer = std::auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &StreamEndpointTest::EnqueueData));
    enqueuer->Start();
    // Wait for the enqueuer to block
    while (wqueue->WriteRequest() == 0) {
        Poll(0);
    }

    wqueue->ShutdownWriter();
    while (true) {
        Poll(0);
        {
            PthreadMutexProtected al(lock);
            if (enqueuedead) { break; }
        }
    }
    enqueuer->Join();

    SetupDescriptors();

    std::auto_ptr<Pthread> dequeuer = std::auto_ptr<Pthread>(
            CreatePthreadFunctional(this, &StreamEndpointTest::DequeueData));
    dequeuer->Start();
    while (true) {
        Poll(0);
        {
            PthreadMutexProtected al(lock);
            if (dequeuedead) { break; }
        }
    }
    dequeuer->Join();
    CPPUNIT_ASSERT(rqueue->IsWriterShutdown());
    CPPUNIT_ASSERT(wqueue->IsWriterShutdown());
    CPPUNIT_ASSERT(numenqueued == numdequeued);
}

void StreamEndpointTest::MaxThreshGrowTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    SetupDescriptors();
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
        Poll(0);
    }

    // Test that growth works
    unsigned len = rqueue->QueueLength() * 2;
    rqueue->Grow(len, maxthresh);
    while (rqueue->QueueLength() != wqueue->QueueLength()) {
        Poll(0);
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
        Poll(0);
    }
    for (unsigned i = 0; i < numchan; ++i) {
        cptr = (const char*)rqueue->GetRawDequeuePtr(maxthresh, i);
        CPPUNIT_ASSERT(cptr);
        CPPUNIT_ASSERT(memcmp(cptr, &buff[maxthresh * i], maxthresh) == 0);
    }
    while (rqueue->MaxThreshold() != wqueue->MaxThreshold()) {
        Poll(0);
    }
    CPPUNIT_ASSERT(wqueue->MaxThreshold() >= maxthresh);
}

void StreamEndpointTest::GrowTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    SetupDescriptors();
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
        Poll(0);
    }

    cptr = (const char*)rqueue->GetRawDequeuePtr(maxthresh, 0);
    unsigned len = wqueue->QueueLength() * 2;
    wqueue->Grow(len, maxthresh);
    while (rqueue->QueueLength() != wqueue->QueueLength()) {
        Poll(0);
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
        Poll(0);
    }
    for (unsigned i = 0; i < numchan; ++i) {
        cptr = (const char*)rqueue->GetRawDequeuePtr(maxthresh, i);
        CPPUNIT_ASSERT(cptr);
        CPPUNIT_ASSERT(memcmp(cptr, &buff[maxthresh * i], maxthresh) == 0);
    }
    while (rqueue->MaxThreshold() != wqueue->MaxThreshold()) {
        Poll(0);
    }
    CPPUNIT_ASSERT(wqueue->MaxThreshold() >= maxthresh);
    wqueue->Enqueue(0);
    rqueue->Dequeue(maxthresh);
    CPPUNIT_ASSERT(rqueue->Count() == 0);
}


int StreamEndpointTest::Poll(double timeout) {
    std::vector<FileHandler*> filehandlers;
    wendp->CheckStatus();
    if (!wendp->Closed()) {
        filehandlers.push_back(wendp.get());
    }
    rendp->CheckStatus();
    if (!rendp->Closed()) {
        filehandlers.push_back(rendp.get());
    }
    if (filehandlers.size() > 0) {
        return FileHandler::Poll(&filehandlers[0], filehandlers.size(), timeout);
    } else { return 0; }
}

void StreamEndpointTest::SetupDescriptors() {
    int fds[2];
    SockHandler::CreatePair(fds);
    rfd->Set(fds[0]);
    rfd->SetDone();
    wfd->Set(fds[1]);
    wfd->SetDone();
}


void StreamEndpointTest::SendWakeup() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
}

LoggerOutput *StreamEndpointTest::GetLogger() {
    return database.get();
}

CPN::shared_ptr<Future<int> > StreamEndpointTest::GetReaderDescriptor(CPN::Key_t readerkey, CPN::Key_t writerkey)
{
    return rfd;
}

CPN::shared_ptr<Future<int> > StreamEndpointTest::GetWriterDescriptor(CPN::Key_t readerkey, CPN::Key_t writerkey)
{
    return wfd;
}
