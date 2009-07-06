

#ifndef BLOCKINGQUEUETEST_H
#define BLOCKINGQUEUETEST_H
#include <cppunit/extensions/HelperMacros.h>
class BlockingQueueTest : public CppUnit::TestFixture {
public:
	void setUp(void);

	void tearDown(void);

	CPPUNIT_TEST_SUITE( BlockingQueueTest );
	CPPUNIT_TEST( test1 );
	CPPUNIT_TEST_SUITE_END();

	void test1(void);
};
#endif
