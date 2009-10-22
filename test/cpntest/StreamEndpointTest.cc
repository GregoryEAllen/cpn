

#include "StreamEndpointTest.h"
#include <cppunit/TestAssert.h>

#include "SimpleQueue.h"
#include "AsyncStream.h"
#include "AsyncSocket.h"

#include <tr1/memory>
#include <vector>

CPPUNIT_TEST_SUITE_REGISTRATION( StreamEndpointTest );

using std::tr1::shared_ptr;
using std::tr1::dynamic_pointer_cast;
using namespace CPN;
using namespace Async;

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
    wqueue = shared_ptr<SimpleQueue>(new SimpleQueue(2*QUEUESIZE,QUEUESIZE,1));
    rqueue = shared_ptr<SimpleQueue>(new SimpleQueue(QUEUESIZE,QUEUESIZE,1));


    Async::SockPtr wsock;
    Async::SockPtr rsock;
    StreamSocket::CreatePair(wsock, rsock);
    wendp = shared_ptr<StreamEndpoint>(new StreamEndpoint(this, RKEY, WKEY, StreamEndpoint::WRITE));
    wendp->SetQueue(wqueue);
    wendp->SetDescriptor(wsock);

    rendp = shared_ptr<StreamEndpoint>(new StreamEndpoint(this, RKEY, WKEY, StreamEndpoint::READ));
    rendp->SetDescriptor(rsock);
    rendp->SetQueue(rqueue);

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

// Test sending some data from the reader
void StreamEndpointTest::EnqueueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    const char data[] = { 'a', 'b', 'c', 'd', 'e' };
    wqueue->RawEnqueue(data, sizeof(data));

    rmh->RMHEnqueue(WKEY, RKEY);

    Msg msg = WaitForMessage();
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(msg.type == RMHENQUEUE);
    CPPUNIT_ASSERT(msg.src == WKEY);
    CPPUNIT_ASSERT(msg.dst == RKEY);

    const char *ptr = (const char*)rqueue->GetRawDequeuePtr(5);
    CPPUNIT_ASSERT(ptr);
    for (unsigned i = 0; i < sizeof(data); ++i) {
        CPPUNIT_ASSERT(ptr[i] == data[i]);
    }
}

// Test sending received data reply
void StreamEndpointTest::DequeueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);

    wmh->WMHDequeue(RKEY, WKEY);

    Msg msg = WaitForMessage();
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(msg.type == WMHDEQUEUE);
    CPPUNIT_ASSERT(msg.src == RKEY);
    CPPUNIT_ASSERT(msg.dst == WKEY);
}

void StreamEndpointTest::BlockTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);

    DEBUG("ReadBlock\n");
    wmh->WMHReadBlock(RKEY, WKEY, 1);
    Msg msg = WaitForMessage();
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(msg.type == WMHREADBLOCK);
    CPPUNIT_ASSERT(msg.src == RKEY);
    CPPUNIT_ASSERT(msg.dst == WKEY);

    // Got to have data in the queue for the block to go through
    // StreamEndpoint first checks to see if it can write some more
    // to the socket, if it can it does, then it checks if requested
    // is available, if so it does nothing more. If not then it queues up
    // a block packet. So we must request here enough to ensure a block
    // packet will be sent.
    DEBUG("WriteBlock\n");
    void *ptr = 0;
    while ((ptr = wqueue->GetRawEnqueuePtr(QUEUESIZE)) != 0) {
        memset(ptr, 0, QUEUESIZE);
        wqueue->Enqueue(QUEUESIZE);
        rmh->RMHEnqueue(WKEY, RKEY);
    }

    rmh->RMHWriteBlock(WKEY, RKEY, QUEUESIZE);
    do {
        msg = WaitForMessage();
    } while (msg.type != RMHWRITEBLOCK);
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(msg.type == RMHWRITEBLOCK);
    CPPUNIT_ASSERT(msg.src == WKEY);
    CPPUNIT_ASSERT(msg.dst == RKEY);
}

void StreamEndpointTest::ThrottleTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    char data[5] = {0};
    CPPUNIT_ASSERT(wqueue->RawEnqueue(&data[0], QUEUESIZE));
    rmh->RMHEnqueue(WKEY, RKEY);
    CPPUNIT_ASSERT(wqueue->RawEnqueue(&data[0], QUEUESIZE));
    rmh->RMHEnqueue(WKEY, RKEY);
    Msg msg = WaitForMessage();
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(msg.type == RMHENQUEUE);
    CPPUNIT_ASSERT(msg.src == WKEY);
    CPPUNIT_ASSERT(msg.dst == RKEY);
    CPPUNIT_ASSERT(rqueue->Count() == QUEUESIZE);
    rqueue->Dequeue(QUEUESIZE);
    wmh->WMHDequeue(RKEY, WKEY);
    for (int i = 2; i > 0; --i) {
        msg = WaitForMessage();
        if (msg.type == RMHENQUEUE) {
            CPPUNIT_ASSERT(msg.type == RMHENQUEUE);
            CPPUNIT_ASSERT(msg.src == WKEY);
            CPPUNIT_ASSERT(msg.dst == RKEY);
        } else if (msg.type == WMHDEQUEUE) {
            CPPUNIT_ASSERT(msg.type == WMHDEQUEUE);
            CPPUNIT_ASSERT(msg.src == RKEY);
            CPPUNIT_ASSERT(msg.dst == WKEY);
        } else {
            CPPUNIT_FAIL("Unknown message type");
        }
    }
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(rqueue->Count() == QUEUESIZE);
}

void StreamEndpointTest::EndOfWriteQueueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    rmh->RMHEndOfWriteQueue(WKEY, RKEY);
    Msg msg = WaitForMessage();
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(msg.type == RMHENDOFWRITEQUEUE);
    CPPUNIT_ASSERT(msg.src == WKEY);
    CPPUNIT_ASSERT(msg.dst == RKEY);
	CPPUNIT_ASSERT_THROW(rmh->RMHEnqueue(WKEY, RKEY), AssertException);
    CPPUNIT_ASSERT(rendp->Shuttingdown());
    CPPUNIT_ASSERT(wendp->Shuttingdown());
}
void StreamEndpointTest::EndOfReadQueueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    wmh->WMHEndOfReadQueue(RKEY, WKEY);
    Msg msg = WaitForMessage();
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(msg.type == WMHENDOFREADQUEUE);
    CPPUNIT_ASSERT(msg.src == RKEY);
    CPPUNIT_ASSERT(msg.dst == WKEY);
    CPPUNIT_ASSERT(rendp->Shuttingdown());
    CPPUNIT_ASSERT(wendp->Shuttingdown());
}



StreamEndpointTest::Msg StreamEndpointTest::WaitForMessage() {
    while (messages.empty()) {
        std::vector<DescriptorPtr> descptrs;
        wendp->RegisterDescriptor(descptrs);
        rendp->RegisterDescriptor(descptrs);
        CPPUNIT_ASSERT(!descptrs.empty());
        CPPUNIT_ASSERT(0 < Descriptor::Poll(descptrs, -1));
    }
    Msg msg = messages.front();
    messages.pop_front();
    return msg;
}

void StreamEndpointTest::RMHEnqueue(Key_t src, Key_t dst) {
    messages.push_back(Msg(RMHENQUEUE, src, dst));
}

void StreamEndpointTest::RMHEndOfWriteQueue(Key_t src, Key_t dst) {
    messages.push_back(Msg(RMHENDOFWRITEQUEUE, src, dst));
}

void StreamEndpointTest::RMHWriteBlock(Key_t src, Key_t dst, unsigned requested) {
    messages.push_back(Msg(RMHWRITEBLOCK, src, dst));
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
}

void StreamEndpointTest::WMHTagChange(Key_t src, Key_t dst) {
    messages.push_back(Msg(WMHTAGCHANGE, src, dst));
}

void StreamEndpointTest::StreamDead(Key_t streamkey) {
}

CPN::weak_ptr<CPN::UnknownStream> StreamEndpointTest::CreateNewQueueStream(CPN::Key_t readerkey, CPN::Key_t writerkey) {
    return CPN::weak_ptr<CPN::UnknownStream>();
}

void StreamEndpointTest::SendWakeup() {
}

const LoggerOutput *StreamEndpointTest::GetLogger() const {
    return &logger;
}

