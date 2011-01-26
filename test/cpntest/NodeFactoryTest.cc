
#include "NodeFactoryTest.h"
#include "MockNodeFactory.h"
#include "MockNode.h"
#include "NodeLoader.h"
#include <cppunit/TestAssert.h>
#include <cstdio>
#include <stdexcept>

CPPUNIT_TEST_SUITE_REGISTRATION( NodeFactoryTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

using CPN::shared_ptr;
using CPN::NodeFactory;
using CPN::NodeLoader;

void NodeFactoryTest::setUp(void) {
}

void NodeFactoryTest::tearDown(void) {
}

/// Test that a factory was stored correctly
void NodeFactoryTest::TestFactoryStore(void) {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    NodeLoader loader;
    NodeFactory *fact = loader.GetFactory(MOCKNODE_TYPENAME);
    CPPUNIT_ASSERT(fact != 0);
}

/// Test that an invalid name returns the expected value
void NodeFactoryTest::TestInvalidName(void) {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    NodeLoader loader;
    NodeFactory *fact = 0;
    CPPUNIT_ASSERT_THROW(loader.GetFactory("AnInvalidName1234556"), std::runtime_error);
    CPPUNIT_ASSERT(fact == 0);
}

void NodeFactoryTest::TestCleanUp(void) {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    shared_ptr<NodeFactory> fact = shared_ptr<NodeFactory>(new MockNodeFactory("Testing12345"));
    NodeLoader loader;
    loader.RegisterFactory(fact);
    CPPUNIT_ASSERT_EQUAL(fact.get(), loader.GetFactory("Testing12345"));
}

