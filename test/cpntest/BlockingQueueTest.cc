
#include "BlockingQueueTest.h"
#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( BlockingQueueTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

void BlockingQueueTest::setUp(void) {
}

void BlockingQueueTest::tearDown(void) {
}

void BlockingQueueTest::test1(void) {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
	CPPUNIT_FAIL("Not Implemented");
}
