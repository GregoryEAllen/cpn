
/*
 * :%s/TESTTEMPLATE_H/NEWNAME_H/g
 * :%s/TestTemplate/NewName/g
 * And add
 * #include <cppunit/TestAssert.h>
 * CPPUNIT_TEST_SUITE_REGISTRATION( NewName );
 * to the source file.
 */
#ifndef TESTTEMPLATE_H
#define TESTTEMPLATE_H
#include <cppunit/extensions/HelperMacros.h>
class TestTemplate : public CppUnit::TestFixture {
public:
	void setUp(void);

	void tearDown(void);

	CPPUNIT_TEST_SUITE( TestTemplate );
	CPPUNIT_TEST( test1 );
	CPPUNIT_TEST_SUITE_END();

	void test1(void);
};
#endif
