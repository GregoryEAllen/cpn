
#include "KernelTest.h"
#include <cppunit/TestAssert.h>
#include "Kernel.h"
#include "MockNodeFactory.h"
#include "MockNode.h"
#include "ThresholdQueueFactory.h"
#include "MockQueueFactory.h"
#include "KernelShutdownException.h"
#include <stdexcept>

CPPUNIT_TEST_SUITE_REGISTRATION( KernelTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

void KernelTest::setUp(void) {
	kernel = new CPN::Kernel(CPN::KernelAttr(1, "test"));
}

void KernelTest::tearDown(void) {
	delete kernel;
	kernel = 0;
}

void KernelTest::TestInvalidNodeCreationType(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	try {
		kernel->CreateNode("test", "invaidname", 0, 0);
		CPPUNIT_FAIL("Created invalid node type.");
	} catch (std::invalid_argument e) {
	}
}

void KernelTest::TestInvalidQueueCreationType(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	try {
		kernel->CreateQueue("test", "invalidname", 0, 0, 0);
		CPPUNIT_FAIL("Created invalid queue type.");
	} catch (std::invalid_argument e) {
	}
}

void KernelTest::TestCreateNodes(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	AddNoOps();
}

void KernelTest::TestStartNoOps(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	AddNoOps();
	kernel->Start();
	kernel->Wait();
}

void KernelTest::TestStartNoOps2(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	kernel->Start();
	kernel->Wait();
	CPPUNIT_ASSERT_THROW(AddNoOps(), CPN::KernelShutdownException);
}

void KernelTest::TestCreateQueues(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	AddDefaultQueues();
	CPNRegisterQueueFactory(MockQueueFactory::GetInstance());
	kernel->CreateQueue("mockq1", QUEUETYPE_MOCKQUEUE, 0, 0, 0);
	kernel->CreateQueue("mockq2", QUEUETYPE_MOCKQUEUE, 0, 0, 0);
	kernel->CreateQueue("mockq3", QUEUETYPE_MOCKQUEUE, 0, 0, 0);
}

void KernelTest::TestConnectQueues(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	AddDefaultQueues();
	AddNoOps();
	kernel->ConnectWriteEndpoint("p", "no op 1", "y");
	kernel->ConnectWriteEndpoint("q", "no op 2", "y");
	kernel->ConnectWriteEndpoint("r", "no op 3", "y");
	kernel->ConnectReadEndpoint("p", "no op 1", "x");
	kernel->ConnectReadEndpoint("q", "no op 2", "x");
	kernel->ConnectReadEndpoint("r", "no op 3", "x");
}

void KernelTest::TestConnectQueuesFailure(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	AddDefaultQueues();
	AddNoOps();
	CPPUNIT_ASSERT_THROW(
			kernel->ConnectWriteEndpoint("invalid queuename", "no op 1", "y"),
		       	std::invalid_argument);
	CPPUNIT_ASSERT_THROW(
			kernel->ConnectWriteEndpoint("q", "invalid nodename", "y"),
		       	std::invalid_argument);
	CPPUNIT_ASSERT_THROW(
			kernel->ConnectReadEndpoint("invalid queuename", "no op 1", "y"),
		       	std::invalid_argument);
	CPPUNIT_ASSERT_THROW(
			kernel->ConnectReadEndpoint("q", "invalid nodename", "y"),
		       	std::invalid_argument);
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


