
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
#include <deque>

class StreamEndpointTest : public CppUnit::TestFixture,
public CPN::ReaderMessageHandler,
public CPN::WriterMessageHandler,
public CPN::KernelMessageHandler {
public:

    StreamEndpointTest() : logger(Logger::INFO) {}

	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( StreamEndpointTest );
	CPPUNIT_TEST( EnqueueTest );
    CPPUNIT_TEST( DequeueTest );
    CPPUNIT_TEST( BlockTest );
    CPPUNIT_TEST( ThrottleTest );
    CPPUNIT_TEST( EndOfWriteQueueTest );
    CPPUNIT_TEST( EndOfReadQueueTest );
	CPPUNIT_TEST_SUITE_END();

	void EnqueueTest();
    void DequeueTest();
    void BlockTest();
    void ThrottleTest();
    void EndOfWriteQueueTest();
    void EndOfReadQueueTest();

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
    void RMHWriteBlock(CPN::Key_t src, CPN::Key_t dst, unsigned requested);
    void RMHTagChange(CPN::Key_t src, CPN::Key_t dst);

    void WMHDequeue(CPN::Key_t src, CPN::Key_t dst);
    void WMHEndOfReadQueue(CPN::Key_t src, CPN::Key_t dst);
    void WMHReadBlock(CPN::Key_t src, CPN::Key_t dst, unsigned requested);
    void WMHTagChange(CPN::Key_t src, CPN::Key_t dst);

    void StreamDead(CPN::Key_t streamkey);
    CPN::weak_ptr<CPN::UnknownStream> CreateNewQueueStream(CPN::Key_t readerkey, CPN::Key_t writerkey);
    void SendWakeup();
    const LoggerOutput *GetLogger() const;

    LoggerStdOutput logger;

    CPN::shared_ptr<CPN::QueueBase> wqueue;
    CPN::shared_ptr<CPN::QueueBase> rqueue;
    CPN::shared_ptr<CPN::StreamEndpoint> wendp;
    CPN::shared_ptr<CPN::StreamEndpoint> rendp;
    ReaderMessageHandler *rmh;
    WriterMessageHandler *wmh;
    std::deque<Msg> messages;
};
#endif
