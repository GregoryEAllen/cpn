
#include "NodeFactoryTest.h"
#include "MockNodeFactory.h"
#include "MockNode.h"
#include <cppunit/TestAssert.h>
#include <cstdio>

CPPUNIT_TEST_SUITE_REGISTRATION( NodeFactoryTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

using CPN::shared_ptr;
using CPN::NodeFactory;

void NodeFactoryTest::setUp(void) {
	CPNRegisterNodeFactory(shared_ptr<NodeFactory>(new MockNodeFactory("MockNode")));
}

void NodeFactoryTest::tearDown(void) {
}

/// Test that a factory was stored correctly
void NodeFactoryTest::TestFactoryStore(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	shared_ptr<NodeFactory> fact = CPNGetNodeFactory("MockNode");
	CPPUNIT_ASSERT(fact.use_count() > 0);
}

/// Test that an invalid name returns the expected value
void NodeFactoryTest::TestInvalidName(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	shared_ptr<NodeFactory> fact = CPNGetNodeFactory("AnInvalidName1234556");
    CPPUNIT_ASSERT(fact.use_count() == 0);
}

void NodeFactoryTest::TestCleanUp(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	shared_ptr<NodeFactory> fact = shared_ptr<NodeFactory>(new MockNodeFactory("Testing12345"));
	CPNRegisterNodeFactory(fact);
	CPPUNIT_ASSERT_EQUAL(fact, CPNGetNodeFactory("Testing12345"));
	CPNUnregisterNodeFactory("Testing12345");
    fact.reset();
	CPPUNIT_ASSERT_EQUAL(fact, CPNGetNodeFactory("Testing12345"));
}

