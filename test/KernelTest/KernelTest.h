
#ifndef CPN_KERNELTEST_H
#define CPN_KERNELTEST_H
#include <cppunit/extensions/HelperMacros.h>
class KernelTest : public CppUnit::TestFixture {
public:
	void setUp(void);

	void tearDown(void);

	CPPUNIT_TEST_SUITE( KernelTest );
	CPPUNIT_TEST( test1 );
	CPPUNIT_TEST_SUITE_END();

	void test1(void);
};
#endif
