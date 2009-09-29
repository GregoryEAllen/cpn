
/*
 * Do the following ex commands
 * :%s/AUTOBUFFERTEST/NEWNAME/g
 * :%s/AutoBufferTest/NewName/g
 * And add
#include "AutoBufferTest.h"
#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( AutoBufferTest );
 * to the source file.
 */
#ifndef AUTOBUFFERTEST_H
#define AUTOBUFFERTEST_H
#pragma once

#include <cppunit/extensions/HelperMacros.h>
class AutoBufferTest : public CppUnit::TestFixture {
public:
	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( AutoBufferTest );
	CPPUNIT_TEST( CircleBufferTest1 );
	CPPUNIT_TEST( CircleBufferTest2 );
	CPPUNIT_TEST( CircleBufferTest3 );
	CPPUNIT_TEST_SUITE_END();

	void CircleBufferTest1();
	void CircleBufferTest2();
	void CircleBufferTest3();
};
#endif
