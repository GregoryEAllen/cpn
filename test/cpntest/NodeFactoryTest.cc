
#include "NodeFactoryTest.h"
#include "MockNodeFactory.h"
#include "MockNode.h"
#include <cppunit/TestAssert.h>
#include <cstdio>

CPPUNIT_TEST_SUITE_REGISTRATION( NodeFactoryTest );


void NodeFactoryTest::setUp(void) {
	CPNRegisterNodeFactory(MockNodeFactory::GetInstance());
}

void NodeFactoryTest::tearDown(void) {
}

/// Test that a factory was stored correctly
void NodeFactoryTest::TestFactoryStore(void) {
	printf("%s\n",__PRETTY_FUNCTION__);
	CPN::NodeFactory* fact = CPNGetNodeFactory("MockNode");
	CPPUNIT_ASSERT(fact != 0);
}

/// Test that an invalid name returns the expected value
void NodeFactoryTest::TestInvalidName(void) {
	printf("%s\n",__PRETTY_FUNCTION__);
	CPN::NodeFactory* fact = CPNGetNodeFactory("AnInvalidName1234556");
	CPN::NodeFactory* fact2 = 0;
	CPPUNIT_ASSERT_EQUAL(fact, fact2);
}

void NodeFactoryTest::TestCleanUp(void) {
	printf("%s\n",__PRETTY_FUNCTION__);
	CPN::NodeFactory* fact = new MockNodeFactory("Testing12345");
	CPNRegisterNodeFactory(fact);
	CPPUNIT_ASSERT_EQUAL(fact, CPNGetNodeFactory("Testing12345"));
	CPNUnregisterNodeFactory("Testing12345");
	delete fact;
	fact = 0;
	CPPUNIT_ASSERT_EQUAL(fact, CPNGetNodeFactory("Testing12345"));
}

