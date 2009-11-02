

#include "StreamEndpointTest.h"
#include <cppunit/TestAssert.h>

#include "CPNSimpleQueue.h"

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
    messages.clear();
}

void StreamEndpointTest::CommunicationTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    // Test sending some data from the reader
    const char data[] = { 'a', 'b', 'c', 'd', 'e' };
    wqueue->RawEnqueue(data, sizeof(data));

    rmh->RMHEnqueue(WKEY, RKEY);

    Msg msg = WaitForMessage();
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(msg.type == RMHENQUEUE);
    CPPUNIT_ASSERT(msg.src == WKEY);
    CPPUNIT_ASSERT(msg.dst == RKEY);

    const void *dequeueptr = rqueue->GetRawDequeuePtr(sizeof(data));
    CPPUNIT_ASSERT(dequeueptr);
    CPPUNIT_ASSERT(memcmp(dequeueptr, data, sizeof(data)));

    // Test sending received data reply

    rqueue->Dequeue(sizeof(data));
    wmh->WMHDequeue(RKEY, WKEY);

    msg = WaitForMessage();
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(msg.type == WMHDEQUEUE);
    CPPUNIT_ASSERT(msg.src == RKEY);
    CPPUNIT_ASSERT(msg.dst == WKEY);

    DEBUG("ReadBlock\n");
    wmh->WMHReadBlock(RKEY, WKEY, 1);
    msg = WaitForMessage();
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(msg.type == WMHREADBLOCK);
    CPPUNIT_ASSERT(msg.src == RKEY);
    CPPUNIT_ASSERT(msg.dst == WKEY);
    CPPUNIT_ASSERT(msg.requested == 1);

    DEBUG("WriteBlock and Throttle\n");
    // Fill up the queue to the brim
    void *enqueueptr = 0;
    while ((enqueueptr = wqueue->GetRawEnqueuePtr(sizeof(data))) != 0) {
        memcpy(enqueueptr, data, sizeof(data));
        wqueue->Enqueue(sizeof(data));
        rmh->RMHEnqueue(WKEY, RKEY);
        while (0 < Poll(0));
    }

    rmh->RMHWriteBlock(WKEY, RKEY, QUEUESIZE);
    do {
        msg = WaitForMessage();
        CPPUNIT_ASSERT(msg.type == RMHWRITEBLOCK || msg.type == RMHENQUEUE);
    } while (msg.type != RMHWRITEBLOCK);
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(msg.type == RMHWRITEBLOCK);
    CPPUNIT_ASSERT(msg.src == WKEY);
    CPPUNIT_ASSERT(msg.dst == RKEY);
    CPPUNIT_ASSERT(msg.requested == QUEUESIZE);

    // Now empty it out
    while ((dequeueptr = rqueue->GetRawDequeuePtr(sizeof(data))) != 0) {
        CPPUNIT_ASSERT(memcmp(dequeueptr, data, sizeof(data)));
        rqueue->Dequeue(sizeof(data));
        wmh->WMHDequeue(RKEY, WKEY);
        while (0 < Poll(0));
    }
    // now send the read block again
    wmh->WMHReadBlock(RKEY, WKEY, sizeof(data));
    do {
        msg = WaitForMessage();
        CPPUNIT_ASSERT(msg.type == WMHREADBLOCK || msg.type == RMHENQUEUE
                || msg.type == WMHDEQUEUE);
    } while (msg.type != WMHREADBLOCK);

    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(wqueue->Empty());
    CPPUNIT_ASSERT(rqueue->Empty());

    // Now send the write shutdown

    rmh->RMHEndOfWriteQueue(WKEY, RKEY);
    while (0 < Poll(0));
    while (!messages.empty()) {
        msg = WaitForMessage();
        CPPUNIT_ASSERT(msg.type == WMHENDOFREADQUEUE || msg.type == RMHENDOFWRITEQUEUE);
    }
    CPPUNIT_ASSERT(wendp->Closed());
    CPPUNIT_ASSERT(wendp->GetStatus() == SocketEndpoint::DEAD);
    CPPUNIT_ASSERT(rendp->Closed());
    CPPUNIT_ASSERT(rendp->GetStatus() == SocketEndpoint::DEAD);
}



StreamEndpointTest::Msg StreamEndpointTest::WaitForMessage() {
    while (messages.empty()) { ASSERT(0 < Poll(-1)); }
    Msg msg = messages.front();
    messages.pop_front();
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
    return FileHandler::Poll(&filehandlers[0], filehandlers.size(), timeout);
}

void StreamEndpointTest::RMHEnqueue(Key_t src, Key_t dst) {
    messages.push_back(Msg(RMHENQUEUE, src, dst));
}

void StreamEndpointTest::RMHEndOfWriteQueue(Key_t src, Key_t dst) {
    messages.push_back(Msg(RMHENDOFWRITEQUEUE, src, dst));
}

void StreamEndpointTest::RMHWriteBlock(Key_t src, Key_t dst, unsigned requested) {
    messages.push_back(Msg(RMHWRITEBLOCK, src, dst));
    messages.back().requested = requested;
}

void StreamEndpointTest::RMHTagChange(Key_t src, Key_t dst) {
    messages.push_back(Msg(RMHTAGCHANGE, src, dst));
}

void StreamEndpointTest::WMHDequeue(Key_t src, Key_t dst) {
    messages.push_back(Msg(WMHDEQUEUE, src, dst));
}

void StreamEndpointTest::WMHEndOfReadQueue(Key_t src, Key_t dst) {
    messages.push_back(Msg(WMHENDOFREADQUEUE, src, dst));
}

void StreamEndpointTest::WMHReadBlock(Key_t src, Key_t dst, unsigned requested) {
    messages.push_back(Msg(WMHREADBLOCK, src, dst));
    messages.back().requested = requested;
}

void StreamEndpointTest::WMHTagChange(Key_t src, Key_t dst) {
    messages.push_back(Msg(WMHTAGCHANGE, src, dst));
}

void StreamEndpointTest::SendWakeup() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
}

const LoggerOutput *StreamEndpointTest::GetLogger() const {
    return &logger;
}

CPN::shared_ptr<Future<int> > StreamEndpointTest::GetReaderDescriptor(CPN::Key_t readerkey, CPN::Key_t writerkey)
{
}

CPN::shared_ptr<Future<int> > StreamEndpointTest::GetWriterDescriptor(CPN::Key_t readerkey, CPN::Key_t writerkey)
{
}

