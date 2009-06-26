
#include "KernelTest.h"
#include <cppunit/TestAssert.h>
#include "Kernel.h"
#include "MockNodeFactory.h"
#include "MockNode.h"
#include "ThresholdQueueFactory.h"
#include <stdexcept>

CPPUNIT_TEST_SUITE_REGISTRATION( KernelTest );

void KernelTest::setUp(void) {
	kernel = new CPN::Kernel(CPN::KernelAttr(1, "test"));
}

void KernelTest::tearDown(void) {
	delete kernel;
	kernel = 0;
}

void KernelTest::TestInvalidNodeCreationType(void) {
	printf("%s\n",__PRETTY_FUNCTION__);
	try {
		kernel->CreateNode("test", "invaidname", 0, 0);
		CPPUNIT_FAIL("Created invalid node type.");
	} catch (std::invalid_argument e) {
	}
}

void KernelTest::TestInvalidQueueCreationType(void) {
	printf("%s\n",__PRETTY_FUNCTION__);
	try {
		kernel->CreateQueue("test", "invalidname", 0, 0, 0);
		CPPUNIT_FAIL("Created invalid queue type.");
	} catch (std::invalid_argument e) {
	}
}

void KernelTest::TestCreateNodes(void) {
	printf("%s\n",__PRETTY_FUNCTION__);
	AddNoOps();
}

void KernelTest::TestStartNoOps(void) {
	printf("%s\n",__PRETTY_FUNCTION__);
	AddNoOps();
	kernel->Start();
}

void KernelTest::TestStartNoOps2(void) {
	printf("%s\n",__PRETTY_FUNCTION__);
	kernel->Start();
	AddNoOps();
}

void KernelTest::TestCreateQueues(void) {
	printf("%s\n",__PRETTY_FUNCTION__);
	AddDefaultQueues();
}

void KernelTest::AddNoOps(void) {
	// Make sure we have access to the given types
	CPNRegisterNodeFactory(MockNodeFactory::GetInstance());
	MockNode::Mode_t mode = MockNode::MODE_NOP;
	// Create some nodes...
	kernel->CreateNode("no op 1", "MockNode", &mode, sizeof(mode));
	kernel->CreateNode("no op 2", "MockNode", &mode, sizeof(mode));
	kernel->CreateNode("no op 3", "MockNode", &mode, sizeof(mode));
}

void KernelTest::AddDefaultQueues(void) {
	CPNRegisterQueueFactory(CPN::ThresholdQueueFactory::GetInstance());
	kernel->CreateQueue("p", CPN_QUEUETYPE_THRESHOLD, 100, 50, 1);
	kernel->CreateQueue("q", CPN_QUEUETYPE_THRESHOLD, 100, 50, 1);
	kernel->CreateQueue("r", CPN_QUEUETYPE_THRESHOLD, 100, 50, 1);
}


