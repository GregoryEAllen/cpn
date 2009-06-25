
#include "QueueFactoryTest.h"
#include <cppunit/TestAssert.h>
#include "QueueFactory.h"
#include "common.h"
#include "MockQueueFactory.h"
#include "MockQueue.h"
#include "ThresholdQueue.h"
#include <cstdio>

CPPUNIT_TEST_SUITE_REGISTRATION( QueueFactoryTest );


void QueueFactoryTest::setUp(void) {
}

void QueueFactoryTest::tearDown(void) {}

void QueueFactoryTest::TestMockStore(void) {
	CPN::QueueFactory* fact = CPN::QueueFactory::GetFactory("MockQueue");
	CPPUNIT_ASSERT(fact != 0);
}

void QueueFactoryTest::TestFactoryStore(void) {
	// Test for the existance of the default queue type
	CPN::QueueFactory* fact = CPN::QueueFactory::GetFactory( CPN::THRESHOLD_QUEUE_TYPE_NAME );
	CPPUNIT_ASSERT(fact != 0);
}

void QueueFactoryTest::TestInvalidName(void) {
	CPN::QueueFactory* fact = CPN::QueueFactory::GetFactory(
			"AnInvalidQueueName;kahsdfgha");
	CPPUNIT_ASSERT(fact == 0);
}

void QueueFactoryTest::TestCleanUp(void) {
	CPN::QueueFactory* fact = new MockQueueFactory("Testing12345");
	CPN::QueueFactory* fact2 = CPN::QueueFactory::GetFactory("Testing12345");
	CPPUNIT_ASSERT_EQUAL(fact, fact2);
	delete fact;
	fact = 0;
	fact2 = CPN::QueueFactory::GetFactory("Testing12345");
	CPPUNIT_ASSERT_EQUAL(fact, fact2);
}

