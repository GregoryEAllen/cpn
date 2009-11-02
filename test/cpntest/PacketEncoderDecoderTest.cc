

#include "PacketEncoderDecoderTest.h"
#include <cppunit/TestAssert.h>
#include <stdlib.h>
#include <stdio.h>

#include "ToString.h"
#include "PacketEncoder.h"
#include "PacketDecoder.h"
#include "CPNSimpleQueue.h"

CPPUNIT_TEST_SUITE_REGISTRATION( PacketEDTest );

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif


using CPN::PacketDecoder;
using CPN::BufferedPacketEncoder;
using CPN::Packet;
using CPN::SimpleQueue;

class MockDecoder : public CPN::PacketDecoder {
public:
    MockDecoder(CPN::QueueBase &queue_, BufferedPacketEncoder &encoder_)
    : numevents(0), queue(queue_), encoder(encoder_)
    {
    }
    unsigned NumEvents() { return numevents; }
    Packet &GetPacket() { return header; }
protected:
    void EnqueuePacket(const Packet &packet) {
        header = packet;
        ++numevents;
        for (unsigned chan = 0; chan < packet.NumChannels(); ++chan) {
            unsigned count = packet.Count();
            void *ptr = queue.GetRawEnqueuePtr(count, chan);
            while (count > 0) {
                unsigned amount = 0;
                const void *src = encoder.GetEncodedBytes(amount);
                unsigned tocopy = 0;
                if (count < amount) { tocopy = count; }
                else { tocopy = amount; }
                memcpy(ptr, src, tocopy);
                encoder.ReleaseEncodedBytes(tocopy);
                count -= tocopy;
                ptr = (char*)ptr + tocopy;
            }
        }
        queue.Enqueue(packet.Count());
    }
    void DequeuePacket(const Packet &packet) {
        header = packet;
        ++numevents;
    }
    void ReadBlockPacket(const Packet &packet) {
        header = packet;
        ++numevents;
    }
    void WriteBlockPacket(const Packet &packet) {
        header = packet;
        ++numevents;
    }
    void EndOfWritePacket(const Packet &packet) {
        header = packet;
        ++numevents;
    }
    void EndOfReadPacket(const Packet &packet) {
        header = packet;
        ++numevents;
    }
    void IDReaderPacket(const Packet &packet) {
        header = packet;
        ++numevents;
    }
    void IDWriterPacket(const Packet &packet) {
        header = packet;
        ++numevents;
    }
private:
    Packet header;
    unsigned numevents;
    CPN::QueueBase &queue;
    BufferedPacketEncoder &encoder;
};

void PacketEDTest::setUp() {
}

void PacketEDTest::tearDown() {
}

void PacketEDTest::EnqueueTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    const unsigned maxlen = 1000;
    const unsigned chans = 10;
    const unsigned step = 100;
    unsigned numpkts = 0;
    SimpleQueue queue(sizeof(int) * maxlen, sizeof(int) * maxlen, chans);
    BufferedPacketEncoder encoder;
    MockDecoder decoder(queue, encoder);
    for (unsigned i = 1; i <= maxlen; i += step) {
        srand(i);
        for (unsigned chan = 0; chan < chans; ++chan) {
            int *ptr = (int*) queue.GetRawEnqueuePtr(sizeof(int)*i, chan);
            for (unsigned j = 0; j < i; ++j) {
                ptr[j] = rand();
            }
        }
        queue.Enqueue(sizeof(int)*i);
        Packet header(sizeof(int)*i*chans, CPN::PACKET_ENQUEUE);
        header.Count(sizeof(int)*i).NumChannels(chans);
        header.BytesQueued(rand()).SourceKey(rand()).DestinationKey(rand())
            .Mode(rand()).Status(rand());
        encoder.SendEnqueue(header, &queue);
        numpkts++;
        Transfer(&encoder, &decoder);
        CPPUNIT_ASSERT(decoder.NumEvents() == numpkts);
        Packet rpacket = decoder.GetPacket();
        CPPUNIT_ASSERT(memcmp(&rpacket, &header, sizeof(Packet)) == 0);

        srand(i);
        for (unsigned chan = 0; chan < chans; ++chan) {
            const int *ptr = (const int*) queue.GetRawDequeuePtr(sizeof(int)*i, chan);
            for (unsigned j = 0; j < i; ++j) {
                CPPUNIT_ASSERT(rand() == ptr[j]);
            }
        }
        queue.Dequeue(sizeof(int)*i);
    }
}

void PacketEDTest::DoTest(Packet &header) {
    const unsigned maxlen = 1000;
    const unsigned chans = 10;
    SimpleQueue queue(sizeof(int) * maxlen, sizeof(int) * maxlen, chans);
    BufferedPacketEncoder encoder;
    MockDecoder decoder(queue, encoder);

    header.Count(rand()).NumChannels(rand());
    header.BytesQueued(rand()).SourceKey(rand()).DestinationKey(rand())
        .Mode(rand()).Status(rand());

    encoder.SendPacket(header);

    Transfer(&encoder, &decoder);

    CPPUNIT_ASSERT(decoder.NumEvents() == 1);
    Packet rpacket = decoder.GetPacket();
    CPPUNIT_ASSERT(memcmp(&rpacket, &header, sizeof(Packet)) == 0);
}

void PacketEDTest::DequeueTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    Packet header(CPN::PACKET_DEQUEUE);
    DoTest(header);
}

void PacketEDTest::ReadBlockTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    Packet header(CPN::PACKET_READBLOCK);
    DoTest(header);
}

void PacketEDTest::WriteBlockTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    Packet header(CPN::PACKET_WRITEBLOCK);
    DoTest(header);
}

void PacketEDTest::EndOfWriteTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    Packet header(CPN::PACKET_ENDOFWRITE);
    DoTest(header);
}

void PacketEDTest::EndOfReadTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    Packet header(CPN::PACKET_ENDOFREAD);
    DoTest(header);
}

void PacketEDTest::ReaderIDTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    Packet header(CPN::PACKET_ID_READER);
    DoTest(header);
}

void PacketEDTest::WriterIDTest() {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    Packet header(CPN::PACKET_ID_WRITER);
    DoTest(header);
}

void PacketEDTest::Transfer(BufferedPacketEncoder *encoder, PacketDecoder *decoder) {
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

