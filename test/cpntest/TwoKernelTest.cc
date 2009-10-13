
#include "TwoKernelTest.h"
#include <cppunit/TestAssert.h>
#include "Kernel.h"
#include "Database.h"
#include "MockNodeFactory.h"
#include "MockNode.h"

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
    CPNRegisterNodeFactory(shared_ptr<MockNodeFactory>(new MockNodeFactory("MockNode")));
    shared_ptr<Database> database = Database::Local();
    KernelAttr kattrone("one");
    kattrone.SetDatabase(database).SetHostName("localhost").SetServName("12345");
    KernelAttr kattrtwo("two");
    kattrtwo.SetDatabase(database).SetHostName("localhost").SetServName("12346");
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

