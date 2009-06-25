
/*
 * :%s/QUEUEFACTORYTEST_H/NEWNAME_H/g
 * :%s/QueueFactoryTest/NewName/g
 * And add
 * to the source file.
 */
#ifndef QUEUEFACTORYTEST_H
#define QUEUEFACTORYTEST_H
#include <cppunit/extensions/HelperMacros.h>
class QueueFactoryTest : public CppUnit::TestFixture {
public:
	void setUp(void);

	void tearDown(void);

	CPPUNIT_TEST_SUITE( QueueFactoryTest );
	CPPUNIT_TEST( TestFactoryStore );
	CPPUNIT_TEST( TestInvalidName );
	CPPUNIT_TEST( TestMockStore );
	CPPUNIT_TEST( TestCleanUp );
	CPPUNIT_TEST( TestCleanUp2 );
	CPPUNIT_TEST_SUITE_END();

	void TestFactoryStore(void);
	void TestMockStore(void);
	void TestInvalidName(void);
	void TestCleanUp(void);
	void TestCleanUp2(void);
};
#endif
