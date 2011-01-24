
#include "KernelTest.h"
#include <cppunit/TestAssert.h>
#include "Kernel.h"
#include "FunctionNode.h"
#include "MockNodeFactory.h"
#include "MockNode.h"
#include "MockSyncNode.h"
#include "VariantCPNLoader.h"
#include "VariantToJSON.h"
#include <stdexcept>
#include <string>
#include <string.h>

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

void KernelTest::setUp() {
}

void KernelTest::tearDown() {
}

void KernelTest::TestInvalidNodeCreationType() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    CPN::Kernel kernel(KernelAttr("test"));
    NodeAttr attr = NodeAttr("invalid", "invalid");
    CPPUNIT_ASSERT_THROW(kernel.CreateNode(attr), std::runtime_error);
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
    kernel.GetDatabase()->UseD4R(false);
    NodeAttr attr("source", MOCKNODE_TYPENAME);
    attr.SetParam(MockNode::GetModeName(MockNode::MODE_SOURCE));
    kernel.CreateNode(attr);
    attr.SetName("sink").SetParam(MockNode::GetModeName(MockNode::MODE_SINK));
    kernel.CreateNode(attr);
    QueueAttr qattr(16, 16);
    qattr.SetDatatype<unsigned long>();
    qattr.SetReader("sink", "x").SetWriter("source", "y");
    kernel.CreateQueue(qattr);
    kernel.WaitNodeTerminate("sink");
    kernel.WaitNodeTerminate("source");
}

void KernelTest::SimpleTwoNodeTestFromVariant() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    Variant args;
    args["name"] = "test";
    args["nodes"][0]["name"] = "source";
    args["nodes"][0]["type"] = MOCKNODE_TYPENAME;
    args["nodes"][0]["param"] = MockNode::GetModeName(MockNode::MODE_SOURCE);
    args["nodes"][1]["name"] = "sink";
    args["nodes"][1]["type"] = MOCKNODE_TYPENAME;
    args["nodes"][1]["param"] = MockNode::GetModeName(MockNode::MODE_SINK);
    args["queues"][0]["size"] = 16;
    args["queues"][0]["threshold"] = 16;
    args["queues"][0]["datatype"] = CPN::TypeName<unsigned long>();
    args["queues"][0]["readernode"] = "sink";
    args["queues"][0]["readerport"] = "x";
    args["queues"][0]["writernode"] = "source";
    args["queues"][0]["writerport"] = "y";

    CPN::Kernel kernel(VariantCPNLoader::GetKernelAttr(args));
    VariantCPNLoader::Setup(&kernel, args);
    kernel.WaitForAllNodeEnd();
}

void KernelTest::AddNoOps(CPN::Kernel &kernel) {

    NodeAttr attr = NodeAttr("no op 1", MOCKNODE_TYPENAME);
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
    kernel.GetDatabase()->UseD4R(false);
    Variant param;
    param["mode"] = MockSyncNode::MODE_SOURCE;
    param["other"] = "sync2";
    NodeAttr attr("sync1", MOCKSYNCNODE_TYPENAME);
    attr.SetParam(VariantToJSON(param));
    kernel.CreateNode(attr);
    param["mode"] = MockSyncNode::MODE_SINK;
    param["other"] = "sync1";
    attr.SetName("sync2").SetParam(VariantToJSON(param));
    kernel.CreateNode(attr);
    kernel.WaitNodeTerminate("sync2");
}

#define SINKNAME "sinkname"
#define SOURCENAME "sourcename"

static void DoSyncTest(void (*fun1)(NodeBase*, std::string),
        void (*fun2)(NodeBase*, std::string)) {

    CPN::Kernel kernel(KernelAttr("test"));
    kernel.GetDatabase()->UseD4R(false);

    kernel.CreateFunctionNode(SOURCENAME, fun1, SINKNAME);
    kernel.CreateFunctionNode(SINKNAME, fun2, SOURCENAME);

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

