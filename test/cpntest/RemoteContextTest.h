
/*
 * Do the following ex commands
 * :%s/REMOTECONTEXTTEST/NEWNAME/g
 * :%s/RemoteContextTest.h/NewName/g
 * And add
#include "RemoteContextTest.h.h"
#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( RemoteContextTest.h );
 * to the source file.
 */
#ifndef REMOTECONTEXTTEST_H
#define REMOTECONTEXTTEST_H
#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "CPNCommon.h"
#include "KernelBase.h"
#include "MockKernel.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include <string>
class LocalRContextServ;

class RemoteContextTest: public CppUnit::TestFixture, public MockKernel {
public:
    void setUp();

    void tearDown();

    CPPUNIT_TEST_SUITE( RemoteContextTest );
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

    LocalRContextServ *serv;

    PthreadMutex lock;
    PthreadCondition cond;
    bool signaled;
};
#endif
