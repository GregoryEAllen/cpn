
#include "TwoKernelTest.h"
#include <cppunit/TestAssert.h>
#include "Kernel.h"
#include "Database.h"
#include "FunctionNode.h"
#include "MockNodeFactory.h"
#include "MockNode.h"
#include "MockSyncNode.h"
#include "ToString.h"

using CPN::Database;
using CPN::Kernel;
using CPN::KernelAttr;
using CPN::shared_ptr;
using CPN::NodeAttr;
using CPN::QueueAttr;
using CPN::FunctionNode;
using CPN::MemberFunction;

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif


CPPUNIT_TEST_SUITE_REGISTRATION( TwoKernelTest );

void TwoKernelTest::setUp() {
    CPNRegisterNodeFactory(shared_ptr<MockNodeFactory>(new MockNodeFactory("MockNode")));
    shared_ptr<Database> database = Database::Local();
    database->LogLevel(50);
    KernelAttr kattrone("one");
    kattrone.SetDatabase(database);
    KernelAttr kattrtwo("two");
    kattrtwo.SetDatabase(database);
    kone = new Kernel(kattrone);
    ktwo = new Kernel(kattrtwo);
}

void TwoKernelTest::tearDown() {
    delete kone;
    kone = 0;
    delete ktwo;
    ktwo = 0;
    CPNUnregisterNodeFactory("MockNode");
}


void TwoKernelTest::SimpleTwoNodeTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    NodeAttr attr("source", "MockNode");
    attr.SetParam(MockNode::GetModeName(MockNode::MODE_SOURCE));
    kone->CreateNode(attr);
    attr.SetName("sink").SetParam(MockNode::GetModeName(MockNode::MODE_SINK));
    ktwo->CreateNode(attr);
    QueueAttr qattr(2*sizeof(unsigned long), 2*sizeof(unsigned long));
    qattr.SetReader("sink", "x").SetWriter("source", "y");
    kone->CreateQueue(qattr);
    kone->WaitNodeTerminate("sink");
    kone->WaitNodeTerminate("source");
}

void TwoKernelTest::TestSync() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    MockSyncNode::RegisterType();
    MockSyncNode::Param param;
    NodeAttr attr("sync1", MOCKSYNCNODE_TYPENAME);
    strncpy(param.othernode, "sync2", 50);
    param.mode = MockSyncNode::MODE_SOURCE;
    attr.SetParam(StaticConstBuffer(&param, sizeof(param)));
    kone->CreateNode(attr);
    strncpy(param.othernode, "sync1", 50);
    param.mode = MockSyncNode::MODE_SINK;
    attr.SetName("sync2").SetParam(StaticConstBuffer(&param, sizeof(param)));
    ktwo->CreateNode(attr);
    ktwo->WaitNodeTerminate("sync2");
}



void TwoKernelTest::DoSyncTest(void (SyncSource::*fun1)(CPN::NodeBase*),
        void (SyncSink::*fun2)(CPN::NodeBase*), unsigned run) {

    std::string sourcename = ToString("source %u", run);
    std::string sinkname = ToString("sink %u", run);
    FunctionNode<MemberFunction<SyncSource> >::RegisterType(sourcename);
    FunctionNode<MemberFunction<SyncSink> >::RegisterType(sinkname);

    // Create local to kone
    NodeAttr attr(sourcename, sourcename);
    SyncSource syncsource = SyncSource(sinkname);
    MemberFunction<SyncSource> memfun1(&syncsource, fun1);
    attr.SetParam(StaticConstBuffer(&memfun1, sizeof(memfun1)));
    kone->CreateNode(attr);

    // Create local to ktwo
    attr.SetName(sinkname).SetTypeName(sinkname);
    SyncSink syncsink = SyncSink(sourcename);
    MemberFunction<SyncSink> memfun2(&syncsink, fun2);
    attr.SetParam(StaticConstBuffer(&memfun2, sizeof(memfun2)));
    ktwo->CreateNode(attr);

    kone->WaitNodeTerminate(sinkname);
    kone->WaitNodeTerminate(sourcename);
}

void TwoKernelTest::TestSyncSourceSink() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    unsigned run = 0;
    DoSyncTest(&SyncSource::Run1, &SyncSink::Run1, run++);
    DoSyncTest(&SyncSource::Run2, &SyncSink::Run1, run++);
    DoSyncTest(&SyncSource::Run3, &SyncSink::Run1, run++);
    DoSyncTest(&SyncSource::Run4, &SyncSink::Run1, run++);

    DoSyncTest(&SyncSource::Run1, &SyncSink::Run2, run++);
    DoSyncTest(&SyncSource::Run2, &SyncSink::Run2, run++);
    DoSyncTest(&SyncSource::Run3, &SyncSink::Run2, run++);
    DoSyncTest(&SyncSource::Run4, &SyncSink::Run2, run++);

    DoSyncTest(&SyncSource::Run5, &SyncSink::Run3, run++);
}

