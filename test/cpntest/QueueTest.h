
/*
 * Do the following ex commands
 * :%s/QUEUETEST/NEWNAME/g
 * :%s/QueueTest/NewName/g
 * And add
#include "QueueTest.h"
#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( QueueTest );
 * to the source file.
 */
#ifndef QUEUETEST_H
#define QUEUETEST_H
#pragma once

#include <cppunit/extensions/HelperMacros.h>

namespace CPN {
    class QueueBase;
}

class QueueTest : public CppUnit::TestFixture {
public:
	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( QueueTest );
	CPPUNIT_TEST( SimpleQueueTest );
	CPPUNIT_TEST( ThresholdQueueTest );
	CPPUNIT_TEST_SUITE_END();

    void SimpleQueueTest();
    void ThresholdQueueTest();

    void TestBulk(CPN::QueueBase *queue);
    void TestDirect(CPN::QueueBase *queue);

};
#endif
