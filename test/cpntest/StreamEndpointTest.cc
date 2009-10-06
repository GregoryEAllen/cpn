

#include "StreamEndpointTest.h"
#include <cppunit/TestAssert.h>

#include "SimpleQueue.h"
#include "AsyncStream.h"

#include <tr1/memory>
#include <vector>

CPPUNIT_TEST_SUITE_REGISTRATION( StreamEndpointTest );

using std::tr1::shared_ptr;
using std::tr1::dynamic_pointer_cast;
using namespace CPN;
using namespace Async;

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

const unsigned QUEUESIZE = 5;

const Key_t RKEY = 1;
const Key_t WKEY = 2;

void Error(int err) {
    CPPUNIT_FAIL("Error on sock");
}

void StreamEndpointTest::setUp() {
    wqueue = shared_ptr<SimpleQueue>(new SimpleQueue(QUEUESIZE,QUEUESIZE,1));
    rqueue = shared_ptr<SimpleQueue>(new SimpleQueue(QUEUESIZE,QUEUESIZE,1));


    StreamSocket::CreatePair(wsock, rsock);
    wsock->ConnectOnError(sigc::ptr_fun(Error));
    rsock->ConnectOnError(sigc::ptr_fun(Error));
    wendp = shared_ptr<StreamEndpoint>(new StreamEndpoint(RKEY, WKEY));
    wendp->SetQueue(wqueue);
    wendp->SetDescriptor(wsock);

    rendp = shared_ptr<StreamEndpoint>(new StreamEndpoint(RKEY, WKEY));
    rendp->SetDescriptor(rsock);
    rendp->SetQueue(rqueue);

    // rmh --> wqueue --> wendp --> sock --> rendp --> rqueue --> tester(rmh)
    // wmh --> rqueue --> rendp --> sock --> wendp --> wqueue --> tester(wmh)
    wmh = rqueue->GetWriterMessageHandler();
    rmh = wqueue->GetReaderMessageHandler();

    wqueue->SetReaderMessageHandler(wendp.get());
    wqueue->SetWriterMessageHandler(this);

    rqueue->SetWriterMessageHandler(rendp.get());
    rqueue->SetReaderMessageHandler(this);
}

void StreamEndpointTest::tearDown() {
    wqueue.reset();
    rqueue.reset();
    wsock.reset();
    rsock.reset();
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

    wmh->WMHReadBlock(RKEY, WKEY, 1);
    Msg msg = WaitForMessage();
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(msg.type == WMHREADBLOCK);
    CPPUNIT_ASSERT(msg.src == RKEY);
    CPPUNIT_ASSERT(msg.dst == WKEY);

    rmh->RMHWriteBlock(WKEY, RKEY, 1);
    msg = WaitForMessage();
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(msg.type == RMHWRITEBLOCK);
    CPPUNIT_ASSERT(msg.src == WKEY);
    CPPUNIT_ASSERT(msg.dst == RKEY);
}

void StreamEndpointTest::ThrottleTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    char data[5] = {0};
    wqueue->RawEnqueue(&data[0], 3);
    rmh->RMHEnqueue(WKEY, RKEY);
    wqueue->RawEnqueue(&data[0], 1);
    rmh->RMHEnqueue(WKEY, RKEY);
    Msg msg = WaitForMessage();
    CPPUNIT_ASSERT(messages.empty());
    CPPUNIT_ASSERT(msg.type == RMHENQUEUE);
    CPPUNIT_ASSERT(msg.src == WKEY);
    CPPUNIT_ASSERT(msg.dst == RKEY);
    CPPUNIT_ASSERT(rqueue->Count() == 3);
    rqueue->Dequeue(3);
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
    CPPUNIT_ASSERT(rqueue->Count() == 1);
}

StreamEndpointTest::Msg StreamEndpointTest::WaitForMessage() {
    std::vector<DescriptorPtr> descptrs;
    descptrs.push_back(wsock);
    descptrs.push_back(rsock);
    while (messages.empty()) {
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

