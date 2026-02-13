
#include "TwoKernelTest.h"
#include <cppunit/TestAssert.h>
#include "Kernel.h"
#include "Context.h"
#include "FunctionNode.h"
#include "MockNodeFactory.h"
#include "MockNode.h"
#include "MockSyncNode.h"
#include "ToString.h"
#include <string.h>

using CPN::Context;
using CPN::Kernel;
using CPN::KernelAttr;
using CPN::shared_ptr;
using CPN::NodeAttr;
using CPN::QueueAttr;

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif


CPPUNIT_TEST_SUITE_REGISTRATION( TwoKernelTest );

void TwoKernelTest::setUp() {
    context = Context::Local();
#ifdef _DEBUG
    context->LogLevel(Logger::TRACE);
#else
    context->LogLevel(Logger::WARNING);
#endif
    KernelAttr kattrone("one");
    kattrone.SetContext(context);
    kattrone.SetRemoteEnabled(true);
    kattrone.UseD4R(false);
    kattrone.SwallowBrokenQueueExceptions(true);
    KernelAttr kattrtwo("two");
    kattrtwo.SetContext(context);
    kattrtwo.SetRemoteEnabled(true);
    kattrtwo.UseD4R(false);
    kattrtwo.SwallowBrokenQueueExceptions(true);
    kone = new Kernel(kattrone);
    ktwo = new Kernel(kattrtwo);
}

void TwoKernelTest::tearDown() {
    delete kone;
    kone = 0;
    delete ktwo;
    ktwo = 0;
    context.reset();
}


void TwoKernelTest::SimpleTwoNodeTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    NodeAttr attr("source", "MockNode");
    attr.SetParam("mode", MockNode::MODE_SOURCE);
    kone->CreateNode(attr);
    attr.SetName("sink").SetParam("mode", MockNode::MODE_SINK);
    ktwo->CreateNode(attr);
    QueueAttr qattr(2*sizeof(unsigned long), 2*sizeof(unsigned long));
    qattr.SetDatatype<unsigned long>();
    qattr.SetReader("sink", "x").SetWriter("source", "y");
    kone->CreateQueue(qattr);
    kone->WaitForNode("sink");
    kone->WaitForNode("source");
}

void TwoKernelTest::TestSync() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    NodeAttr attr("sync1", MOCKSYNCNODE_TYPENAME);
    attr.SetParam("mode", MockSyncNode::MODE_SOURCE);
    attr.SetParam("other", "sync2");
    kone->CreateNode(attr);
    attr.SetName("sync2").SetParam("mode", MockSyncNode::MODE_SINK);
    attr.SetParam("other", "sync1");
    ktwo->CreateNode(attr);
    ktwo->WaitForAllNodes();
}



void TwoKernelTest::DoSyncTest(void (*fun1)(CPN::NodeBase*, std::string),
        void (*fun2)(CPN::NodeBase*, std::string), unsigned run, bool swap) {

    DEBUG(">>DoSyncTest run %u\n", run);
    std::string sourcename = ToString("source %u", run);
    std::string sinkname = ToString("sink %u", run);

    if (!swap) {
        kone->CreateFunctionNode(sourcename, fun1, sinkname);
        ktwo->CreateFunctionNode(sinkname, fun2, sourcename);
    } else {
        ktwo->CreateFunctionNode(sourcename, fun1, sinkname);
        kone->CreateFunctionNode(sinkname, fun2, sourcename);
    }

    kone->WaitForNode(sinkname);
    kone->WaitForNode(sourcename);
}

void TwoKernelTest::TestSyncSourceSink() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    unsigned run = 0;
    bool swap = false;
    for (unsigned i = 0; i < 2; ++i) {
        DoSyncTest(&SyncSource::Run1, &SyncSink::Run1, run++, swap);
        DoSyncTest(&SyncSource::Run2, &SyncSink::Run1, run++, swap);
        DoSyncTest(&SyncSource::Run3, &SyncSink::Run1, run++, swap);
        DoSyncTest(&SyncSource::Run4, &SyncSink::Run1, run++, swap);

        DoSyncTest(&SyncSource::Run1, &SyncSink::Run2, run++, swap);
        DoSyncTest(&SyncSource::Run2, &SyncSink::Run2, run++, swap);
        DoSyncTest(&SyncSource::Run3, &SyncSink::Run2, run++, swap);
        DoSyncTest(&SyncSource::Run4, &SyncSink::Run2, run++, swap);

        DoSyncTest(&SyncSource::Run5, &SyncSink::Run3, run++, swap);

        swap = true;
    }
}

void TwoKernelTest::QueueShutdownTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    NodeAttr attr("source", "MockNode");
    attr.SetParam("mode", MockNode::MODE_SOURCE);
    kone->CreateNode(attr);
    attr.SetName("sink").SetParam("mode", MockNode::MODE_SINK);
    ktwo->CreateNode(attr);
    QueueAttr qattr(2*sizeof(unsigned long), 2*sizeof(unsigned long));
    qattr.SetDatatype<unsigned long>();
    qattr.SetReader("sink", "unused").SetWriter("source", "unused");
    kone->CreateQueue(qattr);
    qattr.SetReader("sink", "x").SetWriter("source", "y");
    kone->CreateQueue(qattr);
    kone->WaitForNode("source");

    ktwo->Terminate();
    ktwo->Wait();
    kone->Terminate();
    kone->Wait();
}

