
/*
 * Do the following ex commands
 * :%s/REMOTEDBTEST/NEWNAME/g
 * :%s/RemoteDBTest.h/NewName/g
 * And add
#include "RemoteDBTest.h.h"
#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( RemoteDBTest.h );
 * to the source file.
 */
#ifndef REMOTEDBTEST_H
#define REMOTEDBTEST_H
#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "CPNCommon.h"
#include "KernelBase.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include <string>
class LocalRDBServ;

class RemoteDBTest: public CppUnit::TestFixture, public CPN::KernelBase {
public:
	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( RemoteDBTest );
	CPPUNIT_TEST( HostSetupTest );
	CPPUNIT_TEST( WaitForHostTest );
	CPPUNIT_TEST( CreateNodeTest );
	CPPUNIT_TEST( WaitForNodeTest );
	CPPUNIT_TEST( ReaderTest );
	CPPUNIT_TEST( WriterTest );
	CPPUNIT_TEST( ConnectTest );
	CPPUNIT_TEST_SUITE_END();

	void HostSetupTest();
    void WaitForHostTest();

    void *WaitForHostSetup();
    std::string m_hostname;
    CPN::Key_t m_hostkey;

    void CreateNodeTest();
    std::string m_nodename;
    CPN::Key_t m_nodekey;
    void WaitForNodeTest();
    void *WaitForNode();

    void ReaderTest();
    void WriterTest();

    void ConnectTest();

    LocalRDBServ *serv;

    PthreadMutex lock;
    PthreadCondition cond;
    bool signaled;
};
#endif
