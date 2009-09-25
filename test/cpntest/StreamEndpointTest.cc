

#include "StreamEndpointTest.h"
#include <cppunit/TestAssert.h>

#include "QueueBase.h"
#include "NodeMessage.h"
#include "MessageQueue.h"

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

void Error(int err) {
    CPPUNIT_FAIL("Error on sock");
}

void StreamEndpointTest::setUp() {
    wqueue = shared_ptr<SimpleQueue>(new SimpleQueue(QUEUESIZE,QUEUESIZE,1));
    rqueue = shared_ptr<SimpleQueue>(new SimpleQueue(QUEUESIZE,QUEUESIZE,1));
    StreamSocket::CreatePair(wsock, rsock);
    wsock->ConnectOnError(sigc::ptr_fun(Error));
    rsock->ConnectOnError(sigc::ptr_fun(Error));
    wendp = shared_ptr<StreamEndpoint>(new StreamEndpoint());
    wendp->SetQueue(wqueue, wqueue->UpStreamChain());
    wendp->SetDescriptor(wsock);
    rendp = shared_ptr<StreamEndpoint>(new StreamEndpoint());
    rendp->SetQueue(rqueue, rqueue->DownStreamChain());
    rendp->SetDescriptor(rsock);
    msgq = MsgQueue<NodeMessagePtr>::Create();
    rqueue->DownStreamChain()->Chain(msgq);
}

void StreamEndpointTest::tearDown() {
    wqueue.reset();
    rqueue.reset();
    wsock.reset();
    rsock.reset();
    wendp.reset();
    rendp.reset();
}

void StreamEndpointTest::EnqueueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    const char data[QUEUESIZE] = { 'a', 'b', 'c', 'd', 'e' };
    wqueue->RawEnqueue(data, sizeof(data));
    SendMessageDownstream(NodeEnqueue::Create(sizeof(data)));

    NodeMessagePtr nodemsg = WaitForMessage();
    CPPUNIT_ASSERT(msgq->Empty());

    NodeEnqueuePtr enqueuemsg = dynamic_pointer_cast<NodeEnqueue>(nodemsg);
    CPPUNIT_ASSERT(enqueuemsg);
    CPPUNIT_ASSERT(enqueuemsg->Amount() == sizeof(data));
    const char *ptr = (const char*)rqueue->GetRawDequeuePtr(5);
    CPPUNIT_ASSERT(ptr);
    for (unsigned i = 0; i < sizeof(data); ++i) {
        CPPUNIT_ASSERT(ptr[i] == data[i]);
    }
}

void StreamEndpointTest::DequeueTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    const unsigned amount = 5;
    SendMessageDownstream(NodeDequeue::Create(amount));
    NodeMessagePtr nodemsg = WaitForMessage();
    CPPUNIT_ASSERT(msgq->Empty());
    NodeDequeuePtr dequeuemsg = dynamic_pointer_cast<NodeDequeue>(nodemsg);
    CPPUNIT_ASSERT(dequeuemsg);
    CPPUNIT_ASSERT(dequeuemsg->Amount() == amount);
}

void StreamEndpointTest::BlockTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    const unsigned request = 5;
    SendMessageDownstream(NodeReadBlock::Create(request));
    NodeMessagePtr nodemsg = WaitForMessage();
    CPPUNIT_ASSERT(msgq->Empty());
    NodeReadBlockPtr nrbptr = dynamic_pointer_cast<NodeReadBlock>(nodemsg);
    CPPUNIT_ASSERT(nrbptr);
    CPPUNIT_ASSERT(nrbptr->Requested() == request);
    SendMessageDownstream(NodeWriteBlock::Create(request));
    nodemsg = WaitForMessage();
    CPPUNIT_ASSERT(msgq->Empty());
    NodeWriteBlockPtr nwbptr = dynamic_pointer_cast<NodeWriteBlock>(nodemsg);
    CPPUNIT_ASSERT(nwbptr);
    CPPUNIT_ASSERT(nwbptr->Requested() == request);
}

void StreamEndpointTest::ThrottleTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    char data[5] = {0};
    wqueue->RawEnqueue(&data[0], 3);
    SendMessageDownstream(NodeEnqueue::Create(3));
    wqueue->RawEnqueue(&data[0], 1);
    SendMessageDownstream(NodeEnqueue::Create(1));
    NodeMessagePtr nodemsg = WaitForMessage();
    CPPUNIT_ASSERT(msgq->Empty());
    SendMessageUpstream(NodeDequeue::Create(3));
    nodemsg = WaitForMessage();
    CPPUNIT_ASSERT(msgq->Empty());

}

NodeMessagePtr StreamEndpointTest::WaitForMessage() {
    std::vector<DescriptorPtr> descptrs;
    descptrs.push_back(wsock);
    descptrs.push_back(rsock);
    while (msgq->Empty()) {
        CPPUNIT_ASSERT(0 < Descriptor::Poll(descptrs, -1));
    }
    return msgq->Get();
}

void StreamEndpointTest::SendMessageDownstream(NodeMessagePtr msg) {
    msg->DispatchOn(wendp.get());
}

void StreamEndpointTest::SendMessageUpstream(CPN::NodeMessagePtr msg) {
    msg->DispatchOn(rendp.get());
}

