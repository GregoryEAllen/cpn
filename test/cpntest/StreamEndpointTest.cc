

#include "StreamEndpointTest.h"
#include <cppunit/TestAssert.h>

#include <tr1/memory>
#include <vector>

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

const Key_t RKEY = 1;
const Key_t WKEY = 2;

void Error(int err) {
}

void StreamEndpointTest::setUp() {

    logger.LogLevel(Logger::WARNING);

    rfd = shared_ptr<FileFuture>(new FileFuture);
    wfd = shared_ptr<FileFuture>(new FileFuture);

    wendp = shared_ptr<SocketEndpoint>(new SocketEndpoint(RKEY, WKEY, SocketEndpoint::WRITE,
                this, QUEUESIZE*2, QUEUESIZE, 1));

    wqueue = wendp;

    rendp = shared_ptr<SocketEndpoint>(new SocketEndpoint(RKEY, WKEY, SocketEndpoint::READ,
                this, QUEUESIZE*2, QUEUESIZE, 1));

    rqueue = rendp;

    // rmh --> wqueue --> wendp --> sock --> rendp --> rqueue --> tester(rmh)
    // wmh --> rqueue --> rendp --> sock --> wendp --> wqueue --> tester(wmh)
    wmh = rqueue->GetWriterMessageHandler();
    rmh = wqueue->GetReaderMessageHandler();

    wqueue->SetWriterMessageHandler(this);

    rqueue->SetReaderMessageHandler(this);
}

void StreamEndpointTest::tearDown() {
    wqueue.reset();
    rqueue.reset();
    wendp.reset();
    rendp.reset();
    readmsg.clear();
    writemsg.clear();
}

void StreamEndpointTest::CommunicationTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    Msg msg;
    SetupDescriptors();
    // Test sending some data from the reader
    const char data[] = { 'a', 'b', 'c', 'd', 'e' };
    wqueue->RawEnqueue(data, sizeof(data));

    rmh->RMHEnqueue(WKEY, RKEY);

    msg = WaitForReadMsg();
    CPPUNIT_ASSERT(msg.src == WKEY);
    CPPUNIT_ASSERT(msg.dst == RKEY);
    CPPUNIT_ASSERT(readmsg.empty());

    // Test sending received data reply
    const void *dequeueptr = rqueue->GetRawDequeuePtr(sizeof(data));
    CPPUNIT_ASSERT(dequeueptr);
    CPPUNIT_ASSERT(memcmp(dequeueptr, data, sizeof(data)) == 0);
    rqueue->Dequeue(sizeof(data));
    wmh->WMHDequeue(RKEY, WKEY);

    msg = WaitForWriteMsg();
    CPPUNIT_ASSERT(writemsg.empty());
    CPPUNIT_ASSERT(msg.type == WMHDEQUEUE);
    CPPUNIT_ASSERT(msg.src == RKEY);
    CPPUNIT_ASSERT(msg.dst == WKEY);

    DEBUG("ReadBlock\n");
    wmh->WMHReadBlock(RKEY, WKEY, 1);
    msg = WaitForWriteMsg();
    CPPUNIT_ASSERT(writemsg.empty());
    CPPUNIT_ASSERT(msg.type == WMHREADBLOCK);
    CPPUNIT_ASSERT(msg.src == RKEY);
    CPPUNIT_ASSERT(msg.dst == WKEY);
    CPPUNIT_ASSERT(msg.requested == 1);

    DEBUG("WriteBlock and Throttle\n");
    // Fill up the queue to the brim
    void *enqueueptr = 0;
    unsigned numsent = 0;
    while ((enqueueptr = wqueue->GetRawEnqueuePtr(sizeof(data))) != 0) {
        ++numsent;
        memcpy(enqueueptr, data, sizeof(data));
        wqueue->Enqueue(sizeof(data));
        rmh->RMHEnqueue(WKEY, RKEY);
        while (0 < Poll(0));
    }

    rmh->RMHWriteBlock(WKEY, RKEY, QUEUESIZE);
    do {
        msg = WaitForReadMsg();
        CPPUNIT_ASSERT(msg.type == RMHWRITEBLOCK || msg.type == RMHENQUEUE);
    } while (msg.type != RMHWRITEBLOCK);
    CPPUNIT_ASSERT(readmsg.empty());
    CPPUNIT_ASSERT(msg.type == RMHWRITEBLOCK);
    CPPUNIT_ASSERT(msg.src == WKEY);
    CPPUNIT_ASSERT(msg.dst == RKEY);
    CPPUNIT_ASSERT(msg.requested == QUEUESIZE);

    // Now empty it out
    unsigned numreceived = 0;
    while ((dequeueptr = rqueue->GetRawDequeuePtr(sizeof(data))) != 0) {
        CPPUNIT_ASSERT(memcmp(dequeueptr, data, sizeof(data)) == 0);
        rqueue->Dequeue(sizeof(data));
        ++numreceived;
        wmh->WMHDequeue(RKEY, WKEY);
        while (0 < Poll(0));
    }
    CPPUNIT_ASSERT(numsent == numreceived);
    // now send the read block again
    wmh->WMHReadBlock(RKEY, WKEY, sizeof(data));
    do {
        msg = WaitForWriteMsg();
        CPPUNIT_ASSERT(msg.type == WMHREADBLOCK || msg.type == WMHDEQUEUE);
    } while (msg.type != WMHREADBLOCK);

    CPPUNIT_ASSERT(writemsg.empty());
    CPPUNIT_ASSERT(wqueue->Empty());
    CPPUNIT_ASSERT(rqueue->Empty());

    // At this point we will have some enqueue messages in the read queue
    // because of throttling
    CPPUNIT_ASSERT(!readmsg.empty());
    readmsg.clear();
    // Now send the write shutdown

    rmh->RMHEndOfWriteQueue(WKEY, RKEY);
    msg = WaitForReadMsg();
    CPPUNIT_ASSERT(msg.type == RMHENDOFWRITEQUEUE);
    msg = WaitForWriteMsg();
    CPPUNIT_ASSERT(msg.type == WMHENDOFREADQUEUE);

    CPPUNIT_ASSERT(wqueue->Empty());
    CPPUNIT_ASSERT(wendp->Closed());
    CPPUNIT_ASSERT(wendp->GetStatus() == SocketEndpoint::DEAD);
    CPPUNIT_ASSERT(rqueue->Empty());
    CPPUNIT_ASSERT(rendp->Closed());
    CPPUNIT_ASSERT(rendp->GetStatus() == SocketEndpoint::DEAD);
}

// Test what happens when we write a bunch of stuff then
// shutdown
void StreamEndpointTest::EndOfWriteQueueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    Msg msg;
    SetupDescriptors();
    const char data[] = { 'a', 'b', 'c', 'd', 'e' };

    void *enqueueptr = 0;
    unsigned numsent = 0;
    while ((enqueueptr = wqueue->GetRawEnqueuePtr(sizeof(data))) != 0) {
        memcpy(enqueueptr, data, sizeof(data));
        wqueue->Enqueue(sizeof(data));
        ++numsent;
        rmh->RMHEnqueue(WKEY, RKEY);
        while (0 < Poll(0));
    }

    rmh->RMHEndOfWriteQueue(WKEY, RKEY);

    const void *dequeueptr;
    unsigned numreceived = 0;
    while ((dequeueptr = rqueue->GetRawDequeuePtr(sizeof(data))) != 0) {
        CPPUNIT_ASSERT(memcmp(dequeueptr, data, sizeof(data)) == 0);
        rqueue->Dequeue(sizeof(data));
        ++numreceived;
        wmh->WMHDequeue(RKEY, WKEY);
        while (0 < Poll(0));
    }
    CPPUNIT_ASSERT(numsent == numreceived);

    do {
        msg = WaitForReadMsg();
        CPPUNIT_ASSERT(msg.type == RMHENDOFWRITEQUEUE || msg.type == RMHENQUEUE);
    } while (msg.type != RMHENDOFWRITEQUEUE);
    CPPUNIT_ASSERT(msg.type == RMHENDOFWRITEQUEUE);
    CPPUNIT_ASSERT(readmsg.empty());
    do {
        msg = WaitForWriteMsg();
        CPPUNIT_ASSERT(msg.type == WMHENDOFREADQUEUE || msg.type == WMHDEQUEUE);
    } while (msg.type != WMHENDOFREADQUEUE);
    CPPUNIT_ASSERT(msg.type == WMHENDOFREADQUEUE);
    CPPUNIT_ASSERT(writemsg.empty());

    CPPUNIT_ASSERT(wqueue->Empty());
    CPPUNIT_ASSERT(wendp->Closed());
    CPPUNIT_ASSERT(wendp->GetStatus() == SocketEndpoint::DEAD);
    CPPUNIT_ASSERT(rqueue->Empty());
    CPPUNIT_ASSERT(rendp->Closed());
    CPPUNIT_ASSERT(rendp->GetStatus() == SocketEndpoint::DEAD);
}

void StreamEndpointTest::EndOfReadQueueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    SetupDescriptors();

    wmh->WMHEndOfReadQueue(RKEY, WKEY);
    Msg msg = WaitForReadMsg();
    CPPUNIT_ASSERT(msg.type == RMHENDOFWRITEQUEUE);
    msg = WaitForWriteMsg();
    CPPUNIT_ASSERT(msg.type == WMHENDOFREADQUEUE);

    CPPUNIT_ASSERT(wendp->Closed());
    CPPUNIT_ASSERT(wendp->GetStatus() == SocketEndpoint::DEAD);
    CPPUNIT_ASSERT(wqueue->Empty());
    CPPUNIT_ASSERT(rendp->Closed());
    CPPUNIT_ASSERT(rendp->GetStatus() == SocketEndpoint::DEAD);
    CPPUNIT_ASSERT(rqueue->Empty());
}

// Test that the connection aborts correctly for end of read
// with data in the queue
void StreamEndpointTest::EndOfReadQueueTest2() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    Msg msg;
    SetupDescriptors();
    const char data[] = { 'a', 'b', 'c', 'd', 'e' };

    void *enqueueptr = 0;
    while ((enqueueptr = wqueue->GetRawEnqueuePtr(sizeof(data))) != 0) {
        memcpy(enqueueptr, data, sizeof(data));
        wqueue->Enqueue(sizeof(data));
        rmh->RMHEnqueue(WKEY, RKEY);
        while (0 < Poll(0));
    }


    wmh->WMHEndOfReadQueue(RKEY, WKEY);

    do {
        msg = WaitForReadMsg();
        CPPUNIT_ASSERT(msg.type == RMHENDOFWRITEQUEUE || msg.type == RMHENQUEUE);
    } while (msg.type != RMHENDOFWRITEQUEUE);
    CPPUNIT_ASSERT(msg.type == RMHENDOFWRITEQUEUE);
    CPPUNIT_ASSERT(readmsg.empty());

    do {
        msg = WaitForWriteMsg();
        CPPUNIT_ASSERT(msg.type == WMHENDOFREADQUEUE || msg.type == WMHDEQUEUE);
    } while (msg.type != WMHENDOFREADQUEUE);
    CPPUNIT_ASSERT(msg.type == WMHENDOFREADQUEUE);
    CPPUNIT_ASSERT(writemsg.empty());

    CPPUNIT_ASSERT(wendp->Closed());
    CPPUNIT_ASSERT(wendp->GetStatus() == SocketEndpoint::DEAD);
    CPPUNIT_ASSERT(!wqueue->Empty());
    CPPUNIT_ASSERT(rendp->Closed());
    CPPUNIT_ASSERT(rendp->GetStatus() == SocketEndpoint::DEAD);
    CPPUNIT_ASSERT(!rqueue->Empty());
}

void StreamEndpointTest::WriteBlockWithNoFDTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    Msg msg;
    const char data[] = { 'a', 'b', 'c', 'd', 'e' };
    void *enqueueptr = 0;
    unsigned numsent = 0;
    while ((enqueueptr = wqueue->GetRawEnqueuePtr(sizeof(data))) != 0) {
        memcpy(enqueueptr, data, sizeof(data));
        wqueue->Enqueue(sizeof(data));
        ++numsent;
        rmh->RMHEnqueue(WKEY, RKEY);
        while (0 < Poll(0));
    }
    rmh->RMHWriteBlock(WKEY, RKEY, QUEUESIZE);

    wmh->WMHReadBlock(RKEY, WKEY, sizeof(data));

    SetupDescriptors();

    while (0 < Poll(0));
    const void *dequeueptr;
    unsigned numreceived = 0;
    while ((dequeueptr = rqueue->GetRawDequeuePtr(sizeof(data))) != 0) {
        CPPUNIT_ASSERT(memcmp(dequeueptr, data, sizeof(data)) == 0);
        rqueue->Dequeue(sizeof(data));
        ++numreceived;
        wmh->WMHDequeue(RKEY, WKEY);
        while (0 < Poll(0));
    }
    CPPUNIT_ASSERT(numsent == numreceived);
    rmh->RMHEndOfWriteQueue(WKEY, RKEY);
    do {
        msg = WaitForReadMsg();
        CPPUNIT_ASSERT(msg.type == RMHENDOFWRITEQUEUE || msg.type == RMHENQUEUE);
    } while (msg.type != RMHENDOFWRITEQUEUE);
    CPPUNIT_ASSERT(msg.type == RMHENDOFWRITEQUEUE);
    CPPUNIT_ASSERT(readmsg.empty());
    do {
        msg = WaitForWriteMsg();
    } while (msg.type != WMHENDOFREADQUEUE);
    CPPUNIT_ASSERT(msg.type == WMHENDOFREADQUEUE);
    CPPUNIT_ASSERT(writemsg.empty());

    CPPUNIT_ASSERT(wqueue->Empty());
    CPPUNIT_ASSERT(wendp->Closed());
    CPPUNIT_ASSERT(wendp->GetStatus() == SocketEndpoint::DEAD);
    CPPUNIT_ASSERT(rqueue->Empty());
    CPPUNIT_ASSERT(rendp->Closed());
    CPPUNIT_ASSERT(rendp->GetStatus() == SocketEndpoint::DEAD);
}

void StreamEndpointTest::WriteEndWithNoFDTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    Msg msg;
    const char data[] = { 'a', 'b', 'c', 'd', 'e' };
    void *enqueueptr = 0;
    unsigned numsent = 0;
    while ((enqueueptr = wqueue->GetRawEnqueuePtr(sizeof(data))) != 0) {
        memcpy(enqueueptr, data, sizeof(data));
        wqueue->Enqueue(sizeof(data));
        ++numsent;
        rmh->RMHEnqueue(WKEY, RKEY);
        while (0 < Poll(0));
    }

    rmh->RMHEndOfWriteQueue(WKEY, RKEY);

    SetupDescriptors();

    while (0 < Poll(0));
    const void *dequeueptr;
    unsigned numreceived = 0;
    while ((dequeueptr = rqueue->GetRawDequeuePtr(sizeof(data))) != 0) {
        CPPUNIT_ASSERT(memcmp(dequeueptr, data, sizeof(data)) == 0);
        rqueue->Dequeue(sizeof(data));
        ++numreceived;
        wmh->WMHDequeue(RKEY, WKEY);
        while (0 < Poll(0));
    }
    CPPUNIT_ASSERT(numsent == numreceived);
    do {
        msg = WaitForReadMsg();
        CPPUNIT_ASSERT(msg.type == RMHENDOFWRITEQUEUE || msg.type == RMHENQUEUE);
    } while (msg.type != RMHENDOFWRITEQUEUE);
    CPPUNIT_ASSERT(msg.type == RMHENDOFWRITEQUEUE);
    CPPUNIT_ASSERT(readmsg.empty());
    do {
        msg = WaitForWriteMsg();
    } while (msg.type != WMHENDOFREADQUEUE);
    CPPUNIT_ASSERT(msg.type == WMHENDOFREADQUEUE);
    CPPUNIT_ASSERT(writemsg.empty());

    CPPUNIT_ASSERT(wqueue->Empty());
    CPPUNIT_ASSERT(wendp->Closed());
    CPPUNIT_ASSERT(wendp->GetStatus() == SocketEndpoint::DEAD);
    CPPUNIT_ASSERT(rqueue->Empty());
    CPPUNIT_ASSERT(rendp->Closed());
    CPPUNIT_ASSERT(rendp->GetStatus() == SocketEndpoint::DEAD);

}

StreamEndpointTest::Msg StreamEndpointTest::WaitForReadMsg() {
    while (readmsg.empty()) { ASSERT(0 <= Poll(-1)); }
    Msg msg = readmsg.front();
    readmsg.pop_front();
    return msg;
}

StreamEndpointTest::Msg StreamEndpointTest::WaitForWriteMsg() {
    while (writemsg.empty()) { ASSERT(0 <= Poll(-1)); }
    Msg msg = writemsg.front();
    writemsg.pop_front();
    return msg;
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

const char* StreamEndpointTest::MsgName(MsgType type) {
    switch(type) {
    case RMHENQUEUE:
        return "Enqueue";
    case RMHENDOFWRITEQUEUE:
        return "End of write";
    case RMHWRITEBLOCK:
        return "Write block";
    case RMHTAGCHANGE:
        return "RMHTagChange";
    case WMHDEQUEUE:
        return "Dequeue";
    case WMHENDOFREADQUEUE:
        return "End of read";
    case WMHREADBLOCK:
        return "Read block";
    case WMHTAGCHANGE:
        return "WMHTagChange";
    }
    CPPUNIT_FAIL("Not reachable");
    return 0;
}

void StreamEndpointTest::RMHEnqueue(Key_t src, Key_t dst) {
    readmsg.push_back(Msg(RMHENQUEUE, src, dst));
}

void StreamEndpointTest::RMHEndOfWriteQueue(Key_t src, Key_t dst) {
    readmsg.push_back(Msg(RMHENDOFWRITEQUEUE, src, dst));
}

void StreamEndpointTest::RMHWriteBlock(Key_t src, Key_t dst, unsigned requested) {
    readmsg.push_back(Msg(RMHWRITEBLOCK, src, dst));
    readmsg.back().requested = requested;
}

void StreamEndpointTest::RMHTagChange(Key_t src, Key_t dst) {
    readmsg.push_back(Msg(RMHTAGCHANGE, src, dst));
}

void StreamEndpointTest::WMHDequeue(Key_t src, Key_t dst) {
    writemsg.push_back(Msg(WMHDEQUEUE, src, dst));
}

void StreamEndpointTest::WMHEndOfReadQueue(Key_t src, Key_t dst) {
    writemsg.push_back(Msg(WMHENDOFREADQUEUE, src, dst));
}

void StreamEndpointTest::WMHReadBlock(Key_t src, Key_t dst, unsigned requested) {
    writemsg.push_back(Msg(WMHREADBLOCK, src, dst));
    writemsg.back().requested = requested;
}

void StreamEndpointTest::WMHTagChange(Key_t src, Key_t dst) {
    writemsg.push_back(Msg(WMHTAGCHANGE, src, dst));
}

void StreamEndpointTest::SendWakeup() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
}

LoggerOutput *StreamEndpointTest::GetLogger() {
    return &logger;
}

CPN::shared_ptr<Future<int> > StreamEndpointTest::GetReaderDescriptor(CPN::Key_t readerkey, CPN::Key_t writerkey)
{
    return rfd;
}

CPN::shared_ptr<Future<int> > StreamEndpointTest::GetWriterDescriptor(CPN::Key_t readerkey, CPN::Key_t writerkey)
{
    return wfd;
}

