
/*
 * Do the following ex commands
 * :%s/MESSAGEQUEUETEST/NEWNAME/g
 * :%s/MsgQueueTest/NewName/g
 * And add
#include "MsgQueueTest.h"
#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( MsgQueueTest );
 * to the source file.
 */
#ifndef MESSAGEQUEUETEST_H
#define MESSAGEQUEUETEST_H
#pragma once

#include <cppunit/extensions/HelperMacros.h>
class MsgQueueTest : public CppUnit::TestFixture {
public:
	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( MsgQueueTest );
	CPPUNIT_TEST( TestMsgQueue );
	CPPUNIT_TEST( TestMsgQueueSignal );
	CPPUNIT_TEST( TestMsgEmptyChain );
	CPPUNIT_TEST( TestMsgChain );
	CPPUNIT_TEST( TestMsgMutator );
	CPPUNIT_TEST( TestMsgBroadcaster );
	CPPUNIT_TEST_SUITE_END();

    void TestMsgQueue();
    void TestMsgQueueSignal();
    void TestMsgEmptyChain();
    void TestMsgChain();
    void TestMsgMutator();
    void TestMsgBroadcaster();
    
    void Callback();
    unsigned counter;
};
#endif
