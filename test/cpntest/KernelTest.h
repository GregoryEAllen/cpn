
#ifndef KERNELTEST_H
#define KERNELTEST_H
#include <cppunit/extensions/HelperMacros.h>
#include "Kernel.h"
class KernelTest : public CppUnit::TestFixture {
public:
	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( KernelTest );
	CPPUNIT_TEST( TestInvalidNodeCreationType );
	CPPUNIT_TEST( TestInvalidQueueCreationType );
	CPPUNIT_TEST( TestCreateNodes );
	CPPUNIT_TEST( SimpleTwoNodeTest );
	CPPUNIT_TEST( TestSync );
	CPPUNIT_TEST( TestSyncSourceSink );
	CPPUNIT_TEST_SUITE_END();

	void TestInvalidNodeCreationType();
	void TestInvalidQueueCreationType();
	void TestCreateNodes();
    void SimpleTwoNodeTest();
    void TestSync();
    void TestSyncSourceSink();


	// Support functions
	void AddNoOps(CPN::Kernel &kernel);

private:
};
#endif
