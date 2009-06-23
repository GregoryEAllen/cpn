
/**
 */
#ifndef TESTTEMPLATE_H
#define TESTTEMPLATE_H
#include <cppunit/extensions/HelperMacros.h>
class NodeFactoryTest : public CppUnit::TestFixture {
public:
	void setUp(void);

	void tearDown(void);

	CPPUNIT_TEST_SUITE( NodeFactoryTest );
	CPPUNIT_TEST( test1 );
	CPPUNIT_TEST_SUITE_END();

	void test1(void);
};
#endif
