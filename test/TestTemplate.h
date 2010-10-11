
/*
 * Do the following ex commands
 * :%s/TESTTEMPLATE/NEWNAME/g
 * :%s/TestTemplate/NewName/g
 * And add
#include "TestTemplate.h"
#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( TestTemplate );
 * to the source file.
 */
#ifndef TESTTEMPLATE_H
#define TESTTEMPLATE_H
#pragma once

#include <cppunit/extensions/HelperMacros.h>
class TestTemplate : public CppUnit::TestFixture {
public:
    void setUp();

    void tearDown();

    CPPUNIT_TEST_SUITE( TestTemplate );
    CPPUNIT_TEST( test1 );
    CPPUNIT_TEST_SUITE_END();

    void test1();
};
#endif
