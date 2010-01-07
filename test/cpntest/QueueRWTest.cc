
#include "QueueRWTest.h"
#include "QueueWriter.h"
#include "QueueReader.h"
#include "KernelBase.h"
#include "QueueBase.h"
#include "SimpleQueue.h"
#include <cppunit/TestAssert.h>
#include <cstdlib>

CPPUNIT_TEST_SUITE_REGISTRATION( QueueRWTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif
/*

using CPN::shared_ptr;
using CPN::QueueReader;
using CPN::QueueWriter;
using CPN::Key_t;
using CPN::QueueBase;
using CPN::NodeMessageHandler;
using CPN::ReaderMessageHandler;
using CPN::WriterMessageHandler;

class MockBlocker : public NodeMessageHandler,
        public ReaderMessageHandler,
        public WriterMessageHandler {
public:
    MockBlocker() {
        writereleased = false;
        readreleased = false;
        writeneedqueue = false;
        readneedqueue = false;
        writekey = 1;
        readkey = 2;
        numtransfer = 100;
        queue = shared_ptr<QueueBase>(new CPN::SimpleQueue(2, 1, 1));
        writer = shared_ptr<QueueWriter>(new QueueWriter(this, this, writekey, readkey, queue));
        reader = shared_ptr<QueueReader>(new QueueReader(this, this, readkey, writekey, queue));
    }

    ~MockBlocker() {
        CPPUNIT_ASSERT(writer.unique());
        CPPUNIT_ASSERT(reader.unique());
        reader->Release();
        writer->Release();
        CPPUNIT_ASSERT(writereleased);
        CPPUNIT_ASSERT(readreleased);
    }

    void RMHEnqueue(Key_t src, Key_t dst) {}
    void RMHEndOfWriteQueue(Key_t src, Key_t dst) {}
    void RMHWriteBlock(Key_t src, Key_t dst, unsigned requested) {}
    void RMHTagChange(Key_t src, Key_t dst) {}

    void WMHDequeue(Key_t src, Key_t dst) {}
    void WMHEndOfReadQueue(Key_t src, Key_t dst) {}
    void WMHReadBlock(Key_t src, Key_t dst, unsigned requested) {}
    void WMHTagChange(Key_t src, Key_t dst) {}

    void Shutdown() {}
    void CreateReader(Key_t readerkey, Key_t writerkey, shared_ptr<QueueBase> q) {}
    void CreateWriter(Key_t readerkey, Key_t writerkey, shared_ptr<QueueBase> q) {}

    void ReadBlock(Key_t readerkey, Key_t writerkey) {
        CPPUNIT_ASSERT(readerkey == readkey);
        CPPUNIT_ASSERT(writerkey == writekey);
        while (writer->Freespace()) {
            char val = (char)rand();
            writer->RawEnqueue(&val, 1);
        }
    }

    void WriteBlock(Key_t writerkey, Key_t readerkey) {
        CPPUNIT_ASSERT(readerkey == readkey);
        CPPUNIT_ASSERT(writerkey == writekey);
        char val;
        while (reader->Count()) {
            reader->RawDequeue(&val, 1);
        }
    }

    void ReleaseWriter(Key_t ekey) {
        CPPUNIT_ASSERT(ekey == writekey);
        writereleased = true;
    }

    void ReleaseReader(Key_t ekey) {
        CPPUNIT_ASSERT(ekey == readkey);
        readreleased = true;
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
};

*/
void QueueRWTest::setUp() {
}

void QueueRWTest::tearDown() {
}

/*
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
*/
