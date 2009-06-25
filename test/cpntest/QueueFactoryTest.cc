
#include "QueueFactoryTest.h"
#include <cppunit/TestAssert.h>
#include "QueueFactory.h"
#include "common.h"
#include "MockQueueFactory.h"
#include "MockQueue.h"
#include "ThresholdQueueFactory.h"
#include <cstdio>

CPPUNIT_TEST_SUITE_REGISTRATION( QueueFactoryTest );


void QueueFactoryTest::setUp(void) {

}

void QueueFactoryTest::tearDown(void) {}

void QueueFactoryTest::TestMockStore(void) {
	CPNRegisterQueueFactory(MockQueueFactory::GetInstance());
	CPN::QueueFactory* fact = CPNGetQueueFactory("MockQueue");
	CPPUNIT_ASSERT(fact != 0);
}

void QueueFactoryTest::TestFactoryStore(void) {
	CPNRegisterQueueFactory(CPN::ThresholdQueueFactory::GetInstance());
	// Test for the existance of the default queue type
	CPN::QueueFactory* fact = CPNGetQueueFactory( CPN_QUEUETYPE_THRESHOLD );
	CPPUNIT_ASSERT(fact != 0);
}

void QueueFactoryTest::TestInvalidName(void) {
	CPN::QueueFactory* fact = CPNGetQueueFactory(
			"AnInvalidQueueName;kahsdfgha");
	CPPUNIT_ASSERT(fact == 0);
}

void QueueFactoryTest::TestCleanUp(void) {
	CPN::QueueFactory* fact = new MockQueueFactory("Testing12345");
	CPNRegisterQueueFactory(fact);
	CPN::QueueFactory* fact2 = CPNGetQueueFactory("Testing12345");
	CPPUNIT_ASSERT_EQUAL(fact, fact2);
	delete fact;
	fact = 0;
	fact2 = CPNGetQueueFactory("Testing12345");
	CPPUNIT_ASSERT_EQUAL(fact, fact2);
}

void QueueFactoryTest::TestCleanUp2(void) {
	CPN::QueueFactory* fact = new MockQueueFactory("Testing12345");
	CPNRegisterQueueFactory(fact);
	CPN::QueueFactory* fact2 = CPNGetQueueFactory("Testing12345");
	CPPUNIT_ASSERT_EQUAL(fact, fact2);
	CPNUnregisterQueueFactory("Testing12345");
	delete fact;
	fact = 0;
	fact2 = CPNGetQueueFactory("Testing12345");
	CPPUNIT_ASSERT_EQUAL(fact, fact2);
}

