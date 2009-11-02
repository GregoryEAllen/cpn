
/*
 * Do the following ex commands
 * :%s/PACKETENCODERDECODERTEST/NEWNAME/g
 * :%s/PacketEDTest/NewName/g
 * And add
#include "PacketEDTest.h"
#include <cppunit/TestAssert.h>
CPPUNIT_TEST_SUITE_REGISTRATION( PacketEDTest );
 * to the source file.
 */
#ifndef PACKETENCODERDECODERTEST_H
#define PACKETENCODERDECODERTEST_H
#pragma once

#include <cppunit/extensions/HelperMacros.h>

namespace CPN {
    class BufferedPacketEncoder;
    class PacketDecoder;
    class Packet;
};
class PacketEDTest : public CppUnit::TestFixture {
public:
	void setUp();

	void tearDown();

	CPPUNIT_TEST_SUITE( PacketEDTest );
	CPPUNIT_TEST( EnqueueTest );
	CPPUNIT_TEST( DequeueTest );
	CPPUNIT_TEST( ReadBlockTest );
	CPPUNIT_TEST( WriteBlockTest );
    CPPUNIT_TEST( EndOfWriteTest );
    CPPUNIT_TEST( EndOfReadTest );
	CPPUNIT_TEST( ReaderIDTest );
	CPPUNIT_TEST( WriterIDTest );
	CPPUNIT_TEST_SUITE_END();

	void EnqueueTest();
	void DequeueTest();

    void ReadBlockTest();
    void WriteBlockTest();

    void EndOfWriteTest();
    void EndOfReadTest();

    void ReaderIDTest();
    void WriterIDTest();

    void Transfer(CPN::BufferedPacketEncoder *encoder, CPN::PacketDecoder *decoder);
    void DoTest(CPN::Packet &header);
};
#endif
