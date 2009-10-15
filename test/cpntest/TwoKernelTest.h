
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
#include "Message.h"
class TwoKernelTest : public CppUnit::TestFixture {
public:
	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( TwoKernelTest );
	CPPUNIT_TEST( SimpleTwoNodeTest );
	//CPPUNIT_TEST( TestSync );
	CPPUNIT_TEST_SUITE_END();

    void SimpleTwoNodeTest();
    void TestSync();

private:
    CPN::Kernel *kone;
    CPN::Kernel *ktwo;
    CPN::shared_ptr<CPN::Database> database;
};
#endif
