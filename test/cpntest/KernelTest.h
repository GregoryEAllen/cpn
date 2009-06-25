
#ifndef KERNELTEST_H
#define KERNELTEST_H
#include <cppunit/extensions/HelperMacros.h>
#include "Kernel.h"
class KernelTest : public CppUnit::TestFixture {
public:
	void setUp(void);

	void tearDown(void);

	CPPUNIT_TEST_SUITE( KernelTest );
	CPPUNIT_TEST( TestInvalidNodeCreationType );
	CPPUNIT_TEST( TestInvalidQueueCreationType );
	CPPUNIT_TEST( TestCreateNodes );
	CPPUNIT_TEST( TestStartNoOps );
	CPPUNIT_TEST( TestStartNoOps2 );
	CPPUNIT_TEST_SUITE_END();

	void TestInvalidNodeCreationType(void);
	void TestInvalidQueueCreationType(void);
	void TestCreateNodes(void);
	void TestStartNoOps(void);
	void TestStartNoOps2(void);

	void AddNoOps(void);

private:
	CPN::Kernel* kernel;
};
#endif
