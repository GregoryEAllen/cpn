
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
#include "PthreadMutex.h"
#include "PthreadCondition.h"

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

    void TestBulk();
    void TestDirect();

    void CommunicationTest();
    void DequeueBlockTest();
    void MaxThreshGrowTest();
    void GrowTest();

    void *EnqueueData();
    void *DequeueData();
    void Reset();

    CPN::QueueBase *queue;

    PthreadMutex enqueue_lock;
    PthreadCondition enqueue_cond;
    bool enqueue_fail;
    bool enqueue_stop;
    bool enqueue_dead;
    unsigned enqueue_num;

    PthreadMutex dequeue_lock;
    PthreadCondition dequeue_cond;
    bool dequeue_fail;
    bool dequeue_stop;
    bool dequeue_dead;
    unsigned dequeue_num;
};
#endif
