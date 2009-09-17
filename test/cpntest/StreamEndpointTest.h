
/*
 * Do the following ex commands
 * :%s/STREAMENDPOINTTEST/NEWNAME/g
 * :%s/StreamEndpointTest/NewName/g
 * And add
#include "StreamEndpointTest.h"
#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( StreamEndpointTest );
 * to the source file.
 */
#ifndef STREAMENDPOINTTEST_H
#define STREAMENDPOINTTEST_H
#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include "StreamEndpoint.h"
#include "AsyncSocket.h"
#include "SimpleQueue.h"

class StreamEndpointTest : public CppUnit::TestFixture {
public:
	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( StreamEndpointTest );
	CPPUNIT_TEST( EnqueueTest );
    CPPUNIT_TEST( DequeueTest );
    CPPUNIT_TEST( BlockTest );
    CPPUNIT_TEST( ThrottleTest );
	CPPUNIT_TEST_SUITE_END();

	void EnqueueTest();
    void DequeueTest();
    void BlockTest();
    void ThrottleTest();

private:

    CPN::NodeMessagePtr WaitForMessage();
    void SendMessageDownstream(CPN::NodeMessagePtr msg);
    void SendMessageUpstream(CPN::NodeMessagePtr msg);
    CPN::shared_ptr<CPN::SimpleQueue> wqueue;
    CPN::shared_ptr<CPN::SimpleQueue> rqueue;
    Async::SockPtr wsock;
    Async::SockPtr rsock;
    CPN::shared_ptr<CPN::StreamEndpoint> wendp;
    CPN::shared_ptr<CPN::StreamEndpoint> rendp;
    CPN::shared_ptr<CPN::MsgQueue<CPN::NodeMessagePtr> > msgq;
};
#endif
