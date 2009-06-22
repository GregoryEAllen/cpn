
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
	TestNodeFactory() : NodeFactory("TestNode") {}

	NodeBase* Create(Kernel &ker, const NodeAttr &attr, const void* const arg,
				const ulong argsize) {
		return new TestNode(ker, attr);
	}

	void Destroy(NodeBase* node) {
		delete node;
	}

};

TestNodeFactory theFactory;

void NodeFactoryTest::setUp(void) {
}

void NodeFactoryTest::tearDown(void) {
}

void NodeFactoryTest::test1(void) {
	NodeFactory* fact = NodeFactory::GetFactory("TestNode");
	NodeFactory* fact2 = &theFactory;
	CPPUNIT_ASSERT_EQUAL(fact, fact2);
}
