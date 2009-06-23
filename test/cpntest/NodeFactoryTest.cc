
#include "NodeFactoryTest.h"
#include "NodeFactory.h"
#include "NodeBase.h"
#include "NodeAttr.h"
#include <cppunit/TestAssert.h>
#include <cstdio>

CPPUNIT_TEST_SUITE_REGISTRATION( NodeFactoryTest );

using namespace CPN;

class TestNode : public NodeBase {
public:
	TestNode(Kernel &ker, const NodeAttr &attr) : NodeBase(ker, attr) {}

	void Process(void) {
		printf("Process");
	}
};

class TestNodeFactory : public NodeFactory {
public:
	TestNodeFactory(std::string name) : NodeFactory(name) {}

	NodeBase* Create(Kernel &ker, const NodeAttr &attr, const void* const arg,
				const ulong argsize) {
		return new TestNode(ker, attr);
	}

	void Destroy(NodeBase* node) {
		delete node;
	}

};

TestNodeFactory theFactory("TestNode");

void NodeFactoryTest::setUp(void) {
}

void NodeFactoryTest::tearDown(void) {
}

/// Test that a factory was stored correctly
void NodeFactoryTest::TestFactoryStore(void) {
	NodeFactory* fact = NodeFactory::GetFactory("TestNode");
	NodeFactory* fact2 = &theFactory;
	CPPUNIT_ASSERT_EQUAL(fact, fact2);
}

/// Test that an invalid name returns the expected value
void NodeFactoryTest::TestInvalidName(void) {
	NodeFactory* fact = NodeFactory::GetFactory("AnInvalidName1234556");
	NodeFactory* fact2 = 0;
	CPPUNIT_ASSERT_EQUAL(fact, fact2);
}

/// Test that on factory deletion the factory is successfully removed
void NodeFactoryTest::TestCleanUp(void) {
	NodeFactory* fact = new TestNodeFactory("Testing12345");
	CPPUNIT_ASSERT_EQUAL(fact, NodeFactory::GetFactory("Testing12345"));
	delete fact;
	fact = 0;
	CPPUNIT_ASSERT_EQUAL(fact, NodeFactory::GetFactory("Testing12345"));
}

