
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
#include "SocketEndpoint.h"
#include "KernelBase.h"
#include "QueueBase.h"
#include <deque>

class StreamEndpointTest : public CppUnit::TestFixture,
public CPN::KernelBase {
public:

    StreamEndpointTest() : logger(Logger::INFO) {}

	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( StreamEndpointTest );
    /*
    CPPUNIT_TEST( CommunicationTest );
    CPPUNIT_TEST( EndOfWriteQueueTest );
    CPPUNIT_TEST( EndOfReadQueueTest );
    CPPUNIT_TEST( EndOfReadQueueTest2 );
    CPPUNIT_TEST( WriteBlockWithNoFDTest );
    CPPUNIT_TEST( WriteEndWithNoFDTest );
    */
	CPPUNIT_TEST_SUITE_END();

    void CommunicationTest();
    void EndOfWriteQueueTest();
    void EndOfReadQueueTest();
    void EndOfReadQueueTest2();
    void WriteBlockWithNoFDTest();
    void WriteEndWithNoFDTest();

private:

    class FileFuture : public Future<int> {
    public:
        FileFuture() : fd(-1), done(false), canceled(false) {}
        bool Done() { return done; }
        void SetDone() { done = true; }
        void Cancel() { canceled = true;}
        bool IsCanceled() { return canceled; }
        int Get() { 
            int ret = fd;
            fd = -1;
            return ret;
        }
        void Set(int fd_) { fd = fd_; }
    private:
        int fd;
        bool done;
        bool canceled;
    };

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
        Msg() {}
        Msg(MsgType t, CPN::Key_t s, CPN::Key_t d)
            : type(t), src(s), dst(d), requested(0) {}
        MsgType type;
        CPN::Key_t src;
        CPN::Key_t dst;
        unsigned requested;
    };

    Msg WaitForReadMsg();
    Msg WaitForWriteMsg();
    int Poll(double timeout);
    void SetupDescriptors();
    const char* MsgName(MsgType type);

    CPN::shared_ptr<Future<int> > GetReaderDescriptor(CPN::Key_t readerkey, CPN::Key_t writerkey);
    CPN::shared_ptr<Future<int> > GetWriterDescriptor(CPN::Key_t readerkey, CPN::Key_t writerkey);
    void SendWakeup();
    LoggerOutput *GetLogger();
    CPN::shared_ptr<CPN::Database> GetDatabase() const { return CPN::shared_ptr<CPN::Database>(); }

    LoggerStdOutput logger;

    CPN::shared_ptr<CPN::QueueBase> wqueue;
    CPN::shared_ptr<CPN::QueueBase> rqueue;
    CPN::shared_ptr<CPN::SocketEndpoint> wendp;
    CPN::shared_ptr<CPN::SocketEndpoint> rendp;
    CPN::shared_ptr<FileFuture> rfd;
    CPN::shared_ptr<FileFuture> wfd;
    std::deque<Msg> readmsg;
    std::deque<Msg> writemsg;
};
#endif
