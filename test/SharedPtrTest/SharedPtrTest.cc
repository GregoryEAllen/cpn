
#include "SharedPtrTest.h"
#include "SharedPtr.h"
#include "ExClass.h"
#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( SharedPtrTest );

class ExClass2 : public ExClass {};

void SharedPtrTest::setUp(void) {
}

void SharedPtrTest::tearDown(void) {
}

void SharedPtrTest::test1(void) {
	SharedPtr<ExClass> sp1;
	SharedPtr<ExClass> sp2(new ExClass());
	SharedPtr<ExClass> sp3;
	sp3 = sp2;
	sp3 = sp3;
	sp1 = sp1;
	sp2 = sp1;
	sp3->func1();
	sp3 = SharedPtr<ExClass>(new ExClass());
	SharedPtr<ExClass2> sp4(new ExClass2());
	sp3 = sp4;
	sp3->func1();
}

