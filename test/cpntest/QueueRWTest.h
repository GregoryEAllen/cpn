
/*
 * Do the following ex commands
 * :%s/QUEUEWRITERTEST/NEWNAME/g
 * :%s/QueueRWTest/NewName/g
 * And add
#include "QueueRWTest.h"
#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( QueueRWTest );
 * to the source file.
 */
#ifndef QUEUERWTEST_H
#define QUEUERWTEST_H
#pragma once

#include <cppunit/extensions/HelperMacros.h>
class QueueRWTest : public CppUnit::TestFixture {
public:
	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( QueueRWTest );
	//CPPUNIT_TEST( WriteTest );
	//CPPUNIT_TEST( ReadTest );
	CPPUNIT_TEST_SUITE_END();

    void WriteTest();
    void ReadTest();
};
#endif
