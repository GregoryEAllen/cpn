#ifndef REMOTEQUEUETEST_H
#define REMOTEQUEUETEST_H
#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "CPNCommon.h"
#include "KernelBase.h"
#include "RemoteQueueHolder.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "Pthread.h"
class RemoteQueueTest : public CppUnit::TestFixture, CPN::KernelBase {
public:
	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( RemoteQueueTest );
    CPPUNIT_TEST( CommunicationTest );
    CPPUNIT_TEST( EndOfWriteQueueTest );
    CPPUNIT_TEST( EndOfReadQueueTest );
    CPPUNIT_TEST( EndOfReadQueueTest2 );
    CPPUNIT_TEST( WriteBlockWithNoFDTest );
    CPPUNIT_TEST( WriteEndWithNoFDTest );
    CPPUNIT_TEST( MaxThreshGrowTest );
    CPPUNIT_TEST( GrowTest );
	CPPUNIT_TEST_SUITE_END();

    void CommunicationTest();
    void EndOfWriteQueueTest();
    void EndOfReadQueueTest();
    void EndOfReadQueueTest2();
    void WriteBlockWithNoFDTest();
    void WriteEndWithNoFDTest();
    void MaxThreshGrowTest();
    void GrowTest();

    void *EnqueueData();
    void *DequeueData();

    void NotifyTerminate();

    void *PollServer();

    CPN::shared_ptr<CPN::ConnectionServer> server;
    CPN::shared_ptr<Pthread> servert;
    CPN::shared_ptr<CPN::RemoteQueue> rendp;
    CPN::shared_ptr<CPN::RemoteQueue> wendp;
    CPN::shared_ptr<CPN::QueueBase> wqueue;
    CPN::shared_ptr<CPN::QueueBase> rqueue;
    CPN::shared_ptr<CPN::Database> database;

    CPN::Key_t hostkey;
    CPN::Key_t nodekey;
    CPN::Key_t writerkey;
    CPN::Key_t readerkey;

    CPN::RemoteQueueHolder remotequeueholder;

    bool fail;
    bool stopenqueue;
    bool stopdequeue;
    bool enqueuedead;
    bool dequeuedead;
    unsigned numenqueued;
    unsigned numdequeued;
    PthreadMutex lock;
    PthreadCondition cond;
};
#endif
