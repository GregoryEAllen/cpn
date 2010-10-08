
#include "TwoKernelTest.h"
#include <cppunit/TestAssert.h>
#include "Kernel.h"
#include "Database.h"
#include "FunctionNode.h"
#include "MockNodeFactory.h"
#include "MockNode.h"
#include "MockSyncNode.h"
#include "ToString.h"
#include "VariantToJSON.h"
#include <string.h>

using CPN::Database;
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
    database = Database::Local();
    database->LogLevel(Logger::WARNING);
    database->UseD4R(false);
    database->SwallowBrokenQueueExceptions(true);
    KernelAttr kattrone("one");
    kattrone.SetDatabase(database);
    kattrone.SetRemoteEnabled(true);
    KernelAttr kattrtwo("two");
    kattrtwo.SetDatabase(database);
    kattrtwo.SetRemoteEnabled(true);
    kone = new Kernel(kattrone);
    ktwo = new Kernel(kattrtwo);
}

void TwoKernelTest::tearDown() {
    delete kone;
    kone = 0;
    delete ktwo;
    ktwo = 0;
    database.reset();
}


void TwoKernelTest::SimpleTwoNodeTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    NodeAttr attr("source", "MockNode");
    attr.SetParam(MockNode::GetModeName(MockNode::MODE_SOURCE));
    kone->CreateNode(attr);
    attr.SetName("sink").SetParam(MockNode::GetModeName(MockNode::MODE_SINK));
    ktwo->CreateNode(attr);
    QueueAttr qattr(2*sizeof(unsigned long), 2*sizeof(unsigned long));
    qattr.SetDatatype<unsigned long>();
    qattr.SetReader("sink", "x").SetWriter("source", "y");
    kone->CreateQueue(qattr);
    kone->WaitNodeTerminate("sink");
    kone->WaitNodeTerminate("source");
}

void TwoKernelTest::TestSync() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    Variant param;
    param["mode"] = MockSyncNode::MODE_SOURCE;
    param["other"] = "sync2";
    NodeAttr attr("sync1", MOCKSYNCNODE_TYPENAME);
    attr.SetParam(VariantToJSON(param));
    kone->CreateNode(attr);
    param["mode"] = MockSyncNode::MODE_SINK;
    param["other"] = "sync1";
    attr.SetName("sync2").SetParam(VariantToJSON(param));
    ktwo->CreateNode(attr);
    ktwo->WaitNodeTerminate("sync2");
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

    kone->WaitNodeTerminate(sinkname);
    kone->WaitNodeTerminate(sourcename);
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
    attr.SetParam(MockNode::GetModeName(MockNode::MODE_SOURCE));
    kone->CreateNode(attr);
    attr.SetName("sink").SetParam(MockNode::GetModeName(MockNode::MODE_SINK));
    ktwo->CreateNode(attr);
    QueueAttr qattr(2*sizeof(unsigned long), 2*sizeof(unsigned long));
    qattr.SetDatatype<unsigned long>();
    qattr.SetReader("sink", "unused").SetWriter("source", "unused");
    kone->CreateQueue(qattr);
    qattr.SetReader("sink", "x").SetWriter("source", "y");
    kone->CreateQueue(qattr);
    kone->WaitNodeTerminate("source");

    ktwo->Terminate();
    ktwo->Wait();
    kone->Terminate();
    kone->Wait();
}

