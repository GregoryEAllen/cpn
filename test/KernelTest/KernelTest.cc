
#include "KernelTest.h"
#include "Kernel.h"

using namespace CPN;

CPPUNIT_TEST_SUITE_REGISTRATION( KernelTest );

void KernelTest::setUp(void) {
}

void KernelTest::tearDown(void) {
}

void KernelTest::test1(void) {
	Kernel kern = Kernel(KernelAttr(1, "1"));
}

