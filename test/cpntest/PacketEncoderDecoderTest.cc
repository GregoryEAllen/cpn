

#include "PacketEncoderDecoderTest.h"
#include <cppunit/TestAssert.h>
#include <stdlib.h>
#include <stdio.h>

#include "ToString.h"
#include "PacketEncoder.h"
#include "PacketDecoder.h"

CPPUNIT_TEST_SUITE_REGISTRATION( PacketEDTest );

using CPN::PacketDecoder;
using CPN::PacketEncoder;

class MockDecoder : public CPN::PacketDecoder {
public:
    MockDecoder()
    :
        dataLength(0),
        numChannels(0),
        queueLength(0),
        queueHint(0),
        maxThreshold(0),
        numEvents(0),
        readerKey(0),
        writerKey(0),
        srcKernelKey(0),
        nodeKey(0)
    {
    }
    void *Data() { return buff.GetBuffer(); }
    unsigned DataLength() { return dataLength; }
    unsigned NumChannels() { return numChannels; }
    unsigned QueueLength() { return queueLength; }
    unsigned QueueHint() { return queueHint; }
    unsigned MaxThreshold() { return maxThreshold; }
    uint64_t ReaderKey() { return readerKey; }
    uint64_t WriterKey() { return writerKey; }
    uint64_t SrcKernelKey() { return srcKernelKey; }
    uint64_t DstKernelKey() { return dstKernelKey; }
    uint64_t NodeKey() { return nodeKey; }
    unsigned NumEvents() { return numEvents; }
    const std::string &NodeName() { return nodename; }
    const std::string &NodeType() { return nodetype; }
    const std::string &Param() { return param; }
protected:
    virtual void ReceivedEnqueue(void *d, unsigned length, unsigned numchans) {
        numEvents++;
        buff.Put(d, length * numchans);
        dataLength = length;
        numChannels = numchans;
    }
    virtual void ReceivedDequeue(unsigned length, unsigned numchans) {
        numEvents++;
        dataLength = length;
        numChannels = numchans;
    }
    virtual void ReceivedReadBlock(unsigned requested) {
        numEvents++;
        dataLength = requested;
    }
    virtual void ReceivedWriteBlock(unsigned requested) {
        numEvents++;
        dataLength = requested;
    }

    virtual void ReceivedCreateReader(
            unsigned qhint, unsigned qlength, unsigned maxthresh,
            unsigned numchans, uint64_t rkey, uint64_t wkey)
    {
        numEvents++;
        queueHint = qhint;
        queueLength = qlength;
        maxThreshold = maxthresh;
        numChannels = numchans;
        readerKey = rkey;
        writerKey = wkey;
    }

    virtual void ReceivedCreateWriter(
            unsigned qhint, unsigned qlength, unsigned maxthresh,
            unsigned numchans, uint64_t rkey, uint64_t wkey)
    {
        numEvents++;
        queueHint = qhint;
        queueLength = qlength;
        maxThreshold = maxthresh;
        numChannels = numchans;
        readerKey = rkey;
        writerKey = wkey;
    }

    virtual void ReceivedCreateQueue(
            unsigned qhint, unsigned qlength, unsigned maxthresh,
            unsigned numchans, uint64_t rkey, uint64_t wkey)
    {
        numEvents++;
        queueHint = qhint;
        queueLength = qlength;
        maxThreshold = maxthresh;
        numChannels = numchans;
        readerKey = rkey;
        writerKey = wkey;
    }

    virtual void ReceivedCreateNode(
            const std::string &nname,
            const std::string &ntype,
            const std::string &prm,
            const StaticConstBuffer arg,
            uint64_t nkey,
            uint64_t hkey)
    {
        numEvents++;
        nodename = nname;
        nodetype = ntype;
        param = prm;
        buff = arg;
        srcKernelKey = hkey;
        nodeKey = nkey;
    }

    virtual void ReceivedReaderID(uint64_t rkey, uint64_t wkey) {
        numEvents++;
        readerKey = rkey;
        writerKey = wkey;
    }

    virtual void ReceivedWriterID(uint64_t wkey, uint64_t rkey) {
        numEvents++;
        writerKey = wkey;
        readerKey = rkey;
    }

    virtual void ReceivedKernelID(uint64_t srckernelkey, uint64_t dstkernelkey) {
        numEvents++;
        srcKernelKey = srckernelkey;
        dstKernelKey = dstkernelkey;
    }
private:
    AutoBuffer buff;
    unsigned dataLength;
    unsigned numChannels;
    unsigned queueLength;
    unsigned queueHint;
    unsigned maxThreshold;
    unsigned numEvents;
    uint64_t readerKey;
    uint64_t writerKey;
    uint64_t srcKernelKey;
    uint64_t dstKernelKey;
    uint64_t nodeKey;
    std::string nodename;
    std::string nodetype;
    std::string param;
};

void PacketEDTest::setUp() {
}

void PacketEDTest::tearDown() {
}

void PacketEDTest::EnqueueTest() {
    MockDecoder decoder;
    PacketEncoder encoder;
    srand(1);
    unsigned maxlen = 1000;
    unsigned chans = 10;
    unsigned step = chans;
    std::vector<int> data(maxlen,0);
    std::vector<const void*> ptrs(chans,0);
    unsigned numpkts = 0;
    for (unsigned i = chans; i <= maxlen; i += step) {
        unsigned len = (i / chans);
        for (unsigned j = 0, p = 0; j < i; ++j) {
            data[j] = rand();
            if ( (j % len) == 0) {
                ptrs[p] = &data[j];
                ++p;
            }
        }
        encoder.SendEnqueue(&ptrs[0], len * sizeof(int), chans);
        numpkts++;
        Transfer(&encoder, &decoder);
        CPPUNIT_ASSERT(decoder.NumEvents() == numpkts);
        CPPUNIT_ASSERT(decoder.NumChannels() == chans);
        CPPUNIT_ASSERT(decoder.DataLength() == len * sizeof(int));
        char *ptr = (char*)decoder.Data();
        for (unsigned p = 0; p < chans; ++p) {
            CPPUNIT_ASSERT(memcmp(ptr, ptrs[p], len *sizeof(int)) == 0);
            ptr += len*sizeof(int);
        }
    }
}

void PacketEDTest::DequeueTest() {
    MockDecoder decoder;
    PacketEncoder encoder;
    encoder.SendDequeue(100, 10);
    Transfer(&encoder, &decoder);
    CPPUNIT_ASSERT(decoder.NumEvents() == 1);
    CPPUNIT_ASSERT(decoder.DataLength() == 100);
    CPPUNIT_ASSERT(decoder.NumChannels() == 10);
}

void PacketEDTest::ReadBlockTest() {
    MockDecoder decoder;
    PacketEncoder encoder;
    encoder.SendReadBlock(100);
    Transfer(&encoder, &decoder);
    CPPUNIT_ASSERT(decoder.NumEvents() == 1);
    CPPUNIT_ASSERT(decoder.DataLength() == 100);
}

void PacketEDTest::WriteBlockTest() {
    MockDecoder decoder;
    PacketEncoder encoder;
    encoder.SendWriteBlock(100);
    Transfer(&encoder, &decoder);
    CPPUNIT_ASSERT(decoder.NumEvents() == 1);
    CPPUNIT_ASSERT(decoder.DataLength() == 100);
}

void PacketEDTest::CreateReaderTest() {
    MockDecoder decoder;
    PacketEncoder encoder;
    srand(1);
    unsigned hint = rand();
    unsigned len = rand();
    unsigned thresh = rand();
    unsigned chans = rand();
    unsigned rkey = rand();
    unsigned wkey = rand();
    encoder.SendCreateReader(hint, len, thresh, chans, rkey, wkey);
    Transfer(&encoder, &decoder);
    CPPUNIT_ASSERT(decoder.NumEvents() == 1);
    CPPUNIT_ASSERT(decoder.QueueHint() == hint);
    CPPUNIT_ASSERT(decoder.QueueLength() == len);
    CPPUNIT_ASSERT(decoder.MaxThreshold() == thresh);
    CPPUNIT_ASSERT(decoder.NumChannels() == chans);
    CPPUNIT_ASSERT(decoder.ReaderKey() == rkey);
    CPPUNIT_ASSERT(decoder.WriterKey() == wkey);
}

void PacketEDTest::CreateWriterTest() {
    MockDecoder decoder;
    PacketEncoder encoder;
    srand(1);
    unsigned hint = rand();
    unsigned len = rand();
    unsigned thresh = rand();
    unsigned chans = rand();
    unsigned rkey = rand();
    unsigned wkey = rand();
    encoder.SendCreateWriter(hint, len, thresh, chans, rkey, wkey);
    Transfer(&encoder, &decoder);
    CPPUNIT_ASSERT(decoder.NumEvents() == 1);
    CPPUNIT_ASSERT(decoder.QueueHint() == hint);
    CPPUNIT_ASSERT(decoder.QueueLength() == len);
    CPPUNIT_ASSERT(decoder.MaxThreshold() == thresh);
    CPPUNIT_ASSERT(decoder.NumChannels() == chans);
    CPPUNIT_ASSERT(decoder.ReaderKey() == rkey);
    CPPUNIT_ASSERT(decoder.WriterKey() == wkey);
}

void PacketEDTest::CreateQueueTest() {
    MockDecoder decoder;
    PacketEncoder encoder;
    srand(1);
    unsigned hint = rand();
    unsigned len = rand();
    unsigned thresh = rand();
    unsigned chans = rand();
    unsigned rkey = rand();
    unsigned wkey = rand();
    encoder.SendCreateQueue(hint, len, thresh, chans, rkey, wkey);
    Transfer(&encoder, &decoder);
    CPPUNIT_ASSERT(decoder.NumEvents() == 1);
    CPPUNIT_ASSERT(decoder.QueueHint() == hint);
    CPPUNIT_ASSERT(decoder.QueueLength() == len);
    CPPUNIT_ASSERT(decoder.MaxThreshold() == thresh);
    CPPUNIT_ASSERT(decoder.NumChannels() == chans);
    CPPUNIT_ASSERT(decoder.ReaderKey() == rkey);
    CPPUNIT_ASSERT(decoder.WriterKey() == wkey);
}

void PacketEDTest::CreateNodeTest() {
    MockDecoder decoder;
    PacketEncoder encoder;
    srand(1);
    std::string nodename = ToString("Node %d", rand());
    std::string nodetype = ToString("type %d", rand());
    std::string param = ToString ("param: %d", rand());
    AutoBuffer arg(sizeof(int)*10);
    for (unsigned i = 0; i < 10; ++i) {
        *((int*)arg.GetBuffer(sizeof(int)*i)) = rand();
    }
    unsigned nodekey = rand();
    unsigned kernelkey = rand();
    encoder.SendCreateNode(nodename, nodetype, param, arg, nodekey, kernelkey);
    Transfer(&encoder, &decoder);
    CPPUNIT_ASSERT(decoder.NumEvents() == 1);
    CPPUNIT_ASSERT(decoder.NodeName() == nodename);
    CPPUNIT_ASSERT(decoder.NodeType() == nodetype);
    CPPUNIT_ASSERT(decoder.Param() == param);
    CPPUNIT_ASSERT(memcmp(arg.GetBuffer(), decoder.Data(), arg.GetSize()) == 0);
    CPPUNIT_ASSERT(decoder.NodeKey() == nodekey);
    CPPUNIT_ASSERT(decoder.SrcKernelKey() == kernelkey);
}

void PacketEDTest::ReaderIDTest() {
    MockDecoder decoder;
    PacketEncoder encoder;
    srand(1);
    unsigned srcid = rand();
    unsigned dstid = rand();
    encoder.SendReaderID(srcid, dstid);
    Transfer(&encoder, &decoder);
    CPPUNIT_ASSERT(decoder.NumEvents() == 1);
    CPPUNIT_ASSERT(decoder.ReaderKey() == srcid);
    CPPUNIT_ASSERT(decoder.WriterKey() == dstid);
}

void PacketEDTest::WriterIDTest() {
    MockDecoder decoder;
    PacketEncoder encoder;
    srand(1);
    unsigned srcid = rand();
    unsigned dstid = rand();
    encoder.SendWriterID(srcid, dstid);
    Transfer(&encoder, &decoder);
    CPPUNIT_ASSERT(decoder.NumEvents() == 1);
    CPPUNIT_ASSERT(decoder.WriterKey() == srcid);
    CPPUNIT_ASSERT(decoder.ReaderKey() == dstid);
}

void PacketEDTest::KernelIDTest() {
    MockDecoder decoder;
    PacketEncoder encoder;
    srand(1);
    unsigned srcid = rand();
    unsigned dstid = rand();
    encoder.SendKernelID(srcid, dstid);
    Transfer(&encoder, &decoder);
    CPPUNIT_ASSERT(decoder.NumEvents() == 1);
    CPPUNIT_ASSERT(decoder.SrcKernelKey() == srcid);
    CPPUNIT_ASSERT(decoder.DstKernelKey() == dstid);
}

void PacketEDTest::Transfer(PacketEncoder *encoder, PacketDecoder *decoder) {
    while (encoder->BytesReady()) {
        unsigned available = 0;
        const void *srcptr = encoder->GetEncodedBytes(available);
        unsigned requested = 0;
        void *dstptr = decoder->GetDecoderBytes(requested);
        unsigned amount = available < requested ? available : requested;
        memcpy(dstptr, srcptr, amount);
        encoder->ReleaseEncodedBytes(amount);
        decoder->ReleaseDecoderBytes(amount);
    }
}

