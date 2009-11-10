
/*
 * Do the following ex commands
 * :%s/STREAMREADERWRITERTEST/NEWNAME/g
 * :%s/TwoKernelTest/NewName/g
 * And add
#include "TwoKernelTest.h"
#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( TwoKernelTest );
 * to the source file.
 */
#ifndef TWOKERNELTEST_H
#define TWOKERNELTEST_H
#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "Kernel.h"
#include "MockSyncNode.h"
class TwoKernelTest : public CppUnit::TestFixture {
public:
    TwoKernelTest() : kone(0), ktwo(0) {}
	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( TwoKernelTest );
	CPPUNIT_TEST( SimpleTwoNodeTest );
	CPPUNIT_TEST( TestSync );
	CPPUNIT_TEST( TestSyncSourceSink );
    CPPUNIT_TEST( QueueShutdownTest );
	CPPUNIT_TEST_SUITE_END();

    void SimpleTwoNodeTest();
    void TestSync();
    void TestSyncSourceSink();
    void QueueShutdownTest();

private:
    void DoSyncTest(void (SyncSource::*fun1)(CPN::NodeBase*),
        void (SyncSink::*fun2)(CPN::NodeBase*), unsigned run,
        bool swap);
    CPN::Kernel *kone;
    CPN::Kernel *ktwo;
    CPN::shared_ptr<CPN::Database> database;
};
#endif
