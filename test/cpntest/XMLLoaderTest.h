
#ifndef XMLLOADERTEST_H
#define XMLLOADERTEST_H
#include <cppunit/extensions/HelperMacros.h>
class XMLLoaderTest : public CppUnit::TestFixture {
public:
	void setUp(void);

	void tearDown(void);

	CPPUNIT_TEST_SUITE( XMLLoaderTest );
	CPPUNIT_TEST( test1 );
	CPPUNIT_TEST_SUITE_END();

	void test1(void);
};
#endif

