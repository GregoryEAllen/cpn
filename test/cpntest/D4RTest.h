

/*
 * Do the following ex commands
 * :%s/D4RTest/NEWNAME/g
 * :%s/D4RTest/NewName/g
 * And add
#include "D4RTest.h"
#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( D4RTest );
 * to the source file.
 */
#ifndef D4RTEST_H
#define D4RTEST_H
#pragma once

#include "D4RTesterBase.h"
#include "CPNCommon.h"
#include "PthreadMutex.h"
#include <cppunit/extensions/HelperMacros.h>
#include <vector>
class D4RTest : public CppUnit::TestFixture, D4R::TesterBase {
public:
	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( D4RTest );
	CPPUNIT_TEST( RunOneKernelTest );
	CPPUNIT_TEST( RunTwoKernelTest );
	CPPUNIT_TEST_SUITE_END();

	void RunTest(int numkernels);
    void RunOneKernelTest();
    void RunTwoKernelTest();

    void Deadlock(D4R::TestNodeBase *tnb);
    void Failure(D4R::TestNodeBase *tnb, const std::string &msg);
    void Complete(D4R::TestNodeBase *tnb);
    void CreateNode(const Variant &noded);
    void CreateQueue(const Variant &queued);

    void Abort();

    PthreadMutex lock;
    unsigned successes;
    CPN::shared_ptr<CPN::Database> database;
    std::vector<CPN::Kernel*> kernels;
};
#endif
