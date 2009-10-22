
#include "KernelTest.h"
#include <cppunit/TestAssert.h>
#include "Kernel.h"
#include "FunctionNode.h"
#include "QueueWriterAdapter.h"
#include "QueueReaderAdapter.h"
#include "MockNodeFactory.h"
#include "MockNode.h"
#include "MockSyncNode.h"
#include <stdexcept>
#include <string>

CPPUNIT_TEST_SUITE_REGISTRATION( KernelTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

using CPN::shared_ptr;
using CPN::Kernel;
using CPN::KernelAttr;
using CPN::NodeAttr;
using CPN::QueueAttr;
using CPN::NodeBase;
using CPN::FunctionNode;
using CPN::MemberFunction;

void KernelTest::setUp() {
    CPNRegisterNodeFactory(shared_ptr<MockNodeFactory>(new MockNodeFactory("MockNode")));
}

void KernelTest::tearDown() {
    CPNUnregisterNodeFactory("MockNode");
}

void KernelTest::TestInvalidNodeCreationType() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    CPN::Kernel kernel(KernelAttr("test"));
    NodeAttr attr = NodeAttr("invalid", "invalid");
	CPPUNIT_ASSERT_THROW(kernel.CreateNode(attr), std::invalid_argument);
}

void KernelTest::TestInvalidQueueCreationType() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    CPN::Kernel kernel(KernelAttr("test"));
    QueueAttr attr = QueueAttr(1024, 1024);
	CPPUNIT_ASSERT_THROW(kernel.CreateQueue(attr), std::invalid_argument);
}

void KernelTest::TestCreateNodes() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    CPN::Kernel kernel(KernelAttr("test"));
	AddNoOps(kernel);
}

void KernelTest::SimpleTwoNodeTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    CPN::Kernel kernel(KernelAttr("test"));
    NodeAttr attr("source", "MockNode");
    attr.SetParam(MockNode::GetModeName(MockNode::MODE_SOURCE));
    kernel.CreateNode(attr);
    attr.SetName("sink").SetParam(MockNode::GetModeName(MockNode::MODE_SINK));
    kernel.CreateNode(attr);
    QueueAttr qattr(16, 16);
    qattr.SetReader("sink", "x").SetWriter("source", "y");
    kernel.CreateQueue(qattr);
    kernel.WaitNodeTerminate("sink");
    kernel.WaitNodeTerminate("source");
}

void KernelTest::AddNoOps(CPN::Kernel &kernel) {

    NodeAttr attr = NodeAttr("no op 1", "MockNode");
    attr.SetParam(MockNode::GetModeName(MockNode::MODE_NOP));
	// Create some nodes...
	kernel.CreateNode(attr);
    attr.SetName("no op 2");
	kernel.CreateNode(attr);
    attr.SetName("no op 3");
	kernel.CreateNode(attr);
}


void KernelTest::TestSync() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    CPN::Kernel kernel(KernelAttr("test"));
    MockSyncNode::RegisterType();
    MockSyncNode::Param param;
    NodeAttr attr("sync1", MOCKSYNCNODE_TYPENAME);
    strncpy(param.othernode, "sync2", 50);
    param.mode = MockSyncNode::MODE_SOURCE;
    attr.SetParam(StaticConstBuffer(&param, sizeof(param)));
    kernel.CreateNode(attr);
    strncpy(param.othernode, "sync1", 50);
    param.mode = MockSyncNode::MODE_SINK;
    attr.SetName("sync2").SetParam(StaticConstBuffer(&param, sizeof(param)));
    kernel.CreateNode(attr);
    kernel.WaitNodeTerminate("sync2");
}

#define SINKNAME "sinkname"
#define SOURCENAME "sourcename"

static void DoSyncTest(void (SyncSource::*fun1)(NodeBase*),
        void (SyncSink::*fun2)(NodeBase*)) {

    CPN::Kernel kernel(KernelAttr("test"));
    FunctionNode<MemberFunction<SyncSource> >::RegisterType(SOURCENAME);
    FunctionNode<MemberFunction<SyncSink> >::RegisterType(SINKNAME);

    NodeAttr attr(SOURCENAME, SOURCENAME);
    SyncSource syncsource = SyncSource(SINKNAME);
    MemberFunction<SyncSource> memfun1(&syncsource, fun1);
    attr.SetParam(StaticConstBuffer(&memfun1, sizeof(memfun1)));
    kernel.CreateNode(attr);

    attr.SetName(SINKNAME).SetTypeName(SINKNAME);
    SyncSink syncsink = SyncSink(SOURCENAME);
    MemberFunction<SyncSink> memfun2(&syncsink, fun2);
    attr.SetParam(StaticConstBuffer(&memfun2, sizeof(memfun2)));
    kernel.CreateNode(attr);

    kernel.WaitNodeTerminate(SINKNAME);
    kernel.WaitNodeTerminate(SOURCENAME);
}

void KernelTest::TestSyncSourceSink() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    DoSyncTest(&SyncSource::Run1, &SyncSink::Run1);
    DoSyncTest(&SyncSource::Run2, &SyncSink::Run1);
    DoSyncTest(&SyncSource::Run3, &SyncSink::Run1);
    DoSyncTest(&SyncSource::Run4, &SyncSink::Run1);

    DoSyncTest(&SyncSource::Run1, &SyncSink::Run2);
    DoSyncTest(&SyncSource::Run2, &SyncSink::Run2);
    DoSyncTest(&SyncSource::Run3, &SyncSink::Run2);
    DoSyncTest(&SyncSource::Run4, &SyncSink::Run2);

    DoSyncTest(&SyncSource::Run5, &SyncSink::Run3);
}

