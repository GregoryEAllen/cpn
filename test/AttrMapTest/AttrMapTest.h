#ifndef ATTRMAPTEST_H
#define ATTRMAPTEST_H
#include <cppunit/extensions/HelperMacros.h>
class AttrMapTest : public CppUnit::TestFixture {
public:
	void setUp(void);

	void tearDown(void);

	CPPUNIT_TEST_SUITE( AttrMapTest );
	CPPUNIT_TEST( test1 );
	CPPUNIT_TEST_SUITE_END();

	void test1(void);
};
#endif
