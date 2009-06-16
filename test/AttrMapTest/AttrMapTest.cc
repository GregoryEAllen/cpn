
#include "AttrMapTest.h"
#include "AttrMap.h"
#include <cppunit/TestAssert.h>

using namespace CppUnit;

CPPUNIT_TEST_SUITE_REGISTRATION( AttrMapTest );
void AttrMapTest::setUp(void) {
}

void AttrMapTest::tearDown(void) {
}

void AttrMapTest::test1(void) {
	AttrMap<int> amap;
	amap.Insert(Attribute(1, "name1"), 2);
	amap.Insert(Attribute(2, "name2"), 3);
	CPPUNIT_ASSERT_EQUAL(amap.Get("name1"), 2);
	CPPUNIT_ASSERT_EQUAL(amap.Get(1), 2);
	CPPUNIT_ASSERT_EQUAL(amap.Get("name1"), amap.Get(1));
	CPPUNIT_ASSERT_EQUAL(amap.Get("name2"), 3);
	CPPUNIT_ASSERT_EQUAL(amap.Get(2), 3);
	CPPUNIT_ASSERT_EQUAL(amap.Get("name2"), amap.Get(2));
	CPPUNIT_ASSERT_EQUAL(amap.Get("Nothing"), 0);
	amap.Clear();
	CPPUNIT_ASSERT_EQUAL(amap.Get("name1"), 0);
}

