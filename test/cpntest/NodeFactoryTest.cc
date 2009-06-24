
#include "NodeFactoryTest.h"
#include "MockNodeFactory.h"
#include "MockNode.h"
#include <cppunit/TestAssert.h>
#include <cstdio>

CPPUNIT_TEST_SUITE_REGISTRATION( NodeFactoryTest );

MockNodeFactory theFactory("TestNode");

void NodeFactoryTest::setUp(void) {
}

void NodeFactoryTest::tearDown(void) {
}

/// Test that a factory was stored correctly
void NodeFactoryTest::TestFactoryStore(void) {
	CPN::NodeFactory* fact = CPN::NodeFactory::GetFactory("TestNode");
	CPN::NodeFactory* fact2 = &theFactory;
	CPPUNIT_ASSERT_EQUAL(fact, fact2);
}

/// Test that an invalid name returns the expected value
void NodeFactoryTest::TestInvalidName(void) {
	CPN::NodeFactory* fact = CPN::NodeFactory::GetFactory("AnInvalidName1234556");
	CPN::NodeFactory* fact2 = 0;
	CPPUNIT_ASSERT_EQUAL(fact, fact2);
}

/// Test that on factory deletion the factory is successfully removed
void NodeFactoryTest::TestCleanUp(void) {
	CPN::NodeFactory* fact = new MockNodeFactory("Testing12345");
	CPPUNIT_ASSERT_EQUAL(fact, CPN::NodeFactory::GetFactory("Testing12345"));
	delete fact;
	fact = 0;
	CPPUNIT_ASSERT_EQUAL(fact, CPN::NodeFactory::GetFactory("Testing12345"));
}

