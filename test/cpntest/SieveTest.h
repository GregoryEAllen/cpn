
/*
 * #include <cppunit/TestAssert.h>
 * CPPUNIT_TEST_SUITE_REGISTRATION( NewName );
 * to the source file.
 */
#ifndef SIEVETEST_H
#define SIEVETEST_H
#include <cppunit/extensions/HelperMacros.h>
class SieveTest : public CppUnit::TestFixture {
public:
    void setUp();

    void tearDown();

    CPPUNIT_TEST_SUITE( SieveTest );
    CPPUNIT_TEST( RunTest );
    CPPUNIT_TEST( RunTwoKernelTest );
    CPPUNIT_TEST_SUITE_END();

    void RunTest();
    void RunTwoKernelTest();

private:
};
#endif
