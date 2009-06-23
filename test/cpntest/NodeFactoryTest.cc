
#include "NodeFactoryTest.h"
#include "NodeFactory.h"
#include "NodeBase.h"
#include "NodeAttr.h"
#include <cppunit/TestAssert.h>
#include <cstdio>

CPPUNIT_TEST_SUITE_REGISTRATION( NodeFactoryTest );

class TestNode : public CPN::NodeBase {
public:
	TestNode(CPN::Kernel &ker, const CPN::NodeAttr &attr) : CPN::NodeBase(ker, attr) {}

	void Process(void) {
		printf("Process");
	}
};

class TestNodeFactory : public CPN::NodeFactory {
public:
	TestNodeFactory(std::string name) : CPN::NodeFactory(name) {}

	CPN::NodeBase* Create(CPN::Kernel &ker, const CPN::NodeAttr &attr, const void* const arg,
				const CPN::ulong argsize) {
		return new TestNode(ker, attr);
	}

	void Destroy(CPN::NodeBase* node) {
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
	CPN::NodeFactory* fact = new TestNodeFactory("Testing12345");
	CPPUNIT_ASSERT_EQUAL(fact, CPN::NodeFactory::GetFactory("Testing12345"));
	delete fact;
	fact = 0;
	CPPUNIT_ASSERT_EQUAL(fact, CPN::NodeFactory::GetFactory("Testing12345"));
}

