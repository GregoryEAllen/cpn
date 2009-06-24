/*
 * And add
 * #include <cppunit/TestAssert.h>
 * CPPUNIT_TEST_SUITE_REGISTRATION( NewName );
 * to the source file.
 */
#ifndef SHAREDPTRTEST_H
#define SHAREDPTRTEST_H
#include <cppunit/extensions/HelperMacros.h>
class SharedPtrTest : public CppUnit::TestFixture {
public:
	void setUp(void);

	void tearDown(void);

	CPPUNIT_TEST_SUITE( SharedPtrTest );
	CPPUNIT_TEST( test1 );
	CPPUNIT_TEST_SUITE_END();

	void test1(void);
};
#endif
