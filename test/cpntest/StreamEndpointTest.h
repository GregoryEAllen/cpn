
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
#include "Message.h"
#include "QueueBase.h"
#include "AsyncSocket.h"
#include <deque>

class StreamEndpointTest : public CppUnit::TestFixture,
public CPN::ReaderMessageHandler,
public CPN::WriterMessageHandler {
public:

	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( StreamEndpointTest );
	CPPUNIT_TEST( EnqueueTest );
    CPPUNIT_TEST( DequeueTest );
    //CPPUNIT_TEST( BlockTest );
    //CPPUNIT_TEST( ThrottleTest );
	CPPUNIT_TEST_SUITE_END();

	void EnqueueTest();
    void DequeueTest();
    void BlockTest();
    void ThrottleTest();

private:

    enum MsgType {
        RMHENQUEUE,
        RMHENDOFWRITEQUEUE,
        RMHWRITEBLOCK,
        RMHTAGCHANGE,
        WMHDEQUEUE,
        WMHENDOFREADQUEUE,
        WMHREADBLOCK,
        WMHTAGCHANGE
    };
    struct Msg {
        Msg(MsgType t, CPN::Key_t s, CPN::Key_t d)
            : type(t), src(s), dst(d) {}
        MsgType type;
        CPN::Key_t src;
        CPN::Key_t dst;
    };

    Msg WaitForMessage();

    void RMHEnqueue(CPN::Key_t src, CPN::Key_t dst);
    void RMHEndOfWriteQueue(CPN::Key_t src, CPN::Key_t dst);
    void RMHWriteBlock(CPN::Key_t src, CPN::Key_t dst);
    void RMHTagChange(CPN::Key_t src, CPN::Key_t dst);

    void WMHDequeue(CPN::Key_t src, CPN::Key_t dst);
    void WMHEndOfReadQueue(CPN::Key_t src, CPN::Key_t dst);
    void WMHReadBlock(CPN::Key_t src, CPN::Key_t dst);
    void WMHTagChange(CPN::Key_t src, CPN::Key_t dst);

    CPN::shared_ptr<CPN::QueueBase> wqueue;
    CPN::shared_ptr<CPN::QueueBase> rqueue;
    Async::SockPtr wsock;
    Async::SockPtr rsock;
    CPN::shared_ptr<CPN::StreamEndpoint> wendp;
    CPN::shared_ptr<CPN::StreamEndpoint> rendp;
    ReaderMessageHandler *rmh;
    WriterMessageHandler *wmh;
    std::deque<Msg> messages;
};
#endif
