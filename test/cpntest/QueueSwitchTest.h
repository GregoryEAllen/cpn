
/*
 * :%s/QUEUESWITCHTEST_H/NEWNAME_H/g
 * :%s/QueueSwitchTest/NewName/g
 * And add
 * #include <cppunit/TestAssert.h>
 * CPPUNIT_TEST_SUITE_REGISTRATION( QueueSwitchTest );
 * to the source file.
 */
#ifndef QUEUESWITCHTEST_H
#define QUEUESWITCHTEST_H
#include <cppunit/extensions/HelperMacros.h>
#include "Kernel.h"
class QueueSwitchTest : public CppUnit::TestFixture {
public:
	void setUp(void);

	void tearDown(void);

	CPPUNIT_TEST_SUITE( QueueSwitchTest );
	CPPUNIT_TEST( ReaderSwitch );
	CPPUNIT_TEST( WriterSwitch );
	CPPUNIT_TEST_SUITE_END();

	void ReaderSwitch(void);
	void WriterSwitch(void);
private:
	CPN::Kernel* kernel;
};
#endif
