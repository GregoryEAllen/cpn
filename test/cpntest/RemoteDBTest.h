
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
#include "Message.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include <string>
class LocalRDBServ;

class RemoteDBTest: public CppUnit::TestFixture, public CPN::KernelMessageHandler {
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


    void CreateWriter(CPN::Key_t dst, const CPN::SimpleQueueAttr &attr);
    void CreateReader(CPN::Key_t dst, const CPN::SimpleQueueAttr &attr);
    void CreateQueue(CPN::Key_t dst, const CPN::SimpleQueueAttr &attr);
    void CreateNode(CPN::Key_t dst, const CPN::NodeAttr &attr);

    const LoggerOutput *GetLogger() const {
        return 0;
    }
    CPN::shared_ptr<CPN::Database> GetDatabase() const {
        return CPN::shared_ptr<CPN::Database>();
    }

    PthreadMutex lock;
    PthreadCondition cond;
    bool signaled;
};
#endif
