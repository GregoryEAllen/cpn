
#include "QueueRWTest.h"
#include "QueueWriter.h"
#include "QueueReader.h"
#include "QueueBlocker.h"
#include "MessageQueue.h"
#include "QueueBase.h"
#include "SimpleQueue.h"
#include <cppunit/TestAssert.h>

CPPUNIT_TEST_SUITE_REGISTRATION( QueueRWTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

using CPN::shared_ptr;
using CPN::QueueBlocker;
using CPN::QueueReader;
using CPN::QueueWriter;
using CPN::Key_t;
using CPN::MsgPut;
using CPN::MsgQueue;
using CPN::NodeMessagePtr;
using CPN::QueueBase;
using CPN::SimpleQueue;

class MockBlocker : public QueueBlocker {
public:
    MockBlocker() {
        msgqueue = MsgQueue<NodeMessagePtr>::Create();
        writereleased = false;
        readreleased = false;
        writeneedqueue = false;
        readneedqueue = false;
        writekey = 1;
        readkey = 2;
        numtransfer = 100;
        queue = shared_ptr<QueueBase>(new SimpleQueue(2, 1, 1));
        writer = shared_ptr<QueueWriter>(new QueueWriter(this, writekey));
        reader = shared_ptr<QueueReader>(new QueueReader(this, readkey));
    }

    ~MockBlocker() {
        CPPUNIT_ASSERT(writer.unique());
        CPPUNIT_ASSERT(reader.unique());
        reader->Release();
        writer->Release();
        CPPUNIT_ASSERT(writereleased);
        CPPUNIT_ASSERT(readreleased);
    }

    void ReadBlock(shared_ptr<QueueReader> r, unsigned thresh) {
        CPPUNIT_ASSERT(r == reader);
        while (writer->Freespace()) {
            char val = (char)rand();
            writer->RawEnqueue(&val, 1);
        }
    }

    void WriteBlock(shared_ptr<QueueWriter> w, unsigned thresh) {
        CPPUNIT_ASSERT(w == writer);
        char val;
        while (reader->Count()) {
            reader->RawDequeue(&val, 1);
        }
    }

    void ReadNeedQueue(Key_t ekey) {
        CPPUNIT_ASSERT(readkey == ekey);
        if (readneedqueue) {
            reader->SetQueue(queue);
        }
        readneedqueue = true;
    }

    void WriteNeedQueue(Key_t ekey) {
        CPPUNIT_ASSERT(ekey == writekey);
        if (writeneedqueue) {
            writer->SetQueue(queue);
        }
        writeneedqueue = true;
    }

    void ReleaseWriter(Key_t ekey) {
        CPPUNIT_ASSERT(ekey == writekey);
        writereleased = true;
    }

    void ReleaseReader(Key_t ekey) {
        CPPUNIT_ASSERT(ekey == readkey);
        readreleased = true;
    }

    shared_ptr<MsgPut<NodeMessagePtr> > GetMsgPut() {
        return msgqueue;
    }

    void DoWriteTest() {
        srand(1);
        writeblocks = true;
        readblocks = false;
        for (unsigned i = 0; i < numtransfer; ++i) {
            char val = (char)rand();
            writer->RawEnqueue(&val, 1);
        }
    }

    void DoReadTest() {
        srand(1);
        writeblocks = false;
        readblocks = true;
        for (unsigned i = 0; i < numtransfer; ++i) {
            char val;
            reader->RawDequeue(&val, 1);
        }
    }

    void CheckTerminate() {
    }

    shared_ptr<QueueBase> queue;
    shared_ptr<QueueWriter> writer;
    shared_ptr<QueueReader> reader;
    Key_t writekey;
    Key_t readkey;
    unsigned numtransfer;
    bool writeblocks;
    bool readblocks;
    bool writeneedqueue;
    bool readneedqueue;
    bool writereleased;
    bool readreleased;
    shared_ptr<MsgQueue<NodeMessagePtr> > msgqueue;
};

void QueueRWTest::setUp() {
}

void QueueRWTest::tearDown() {
}

void QueueRWTest::WriteTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    MockBlocker blocker;
    blocker.DoWriteTest();
}

void QueueRWTest::ReadTest() {
	DEBUG("%s\n",__PRETTY_FUNCTION__);
    MockBlocker blocker;
    blocker.DoReadTest();
}

