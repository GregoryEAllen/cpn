
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
    class PacketEncoder;
    class PacketDecoder;
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
	CPPUNIT_TEST( CreateReaderTest );
	CPPUNIT_TEST( CreateWriterTest );
	CPPUNIT_TEST( CreateQueueTest );
	CPPUNIT_TEST( CreateNodeTest );
	CPPUNIT_TEST( ReaderIDTest );
	CPPUNIT_TEST( WriterIDTest );
	CPPUNIT_TEST( KernelIDTest );
	CPPUNIT_TEST_SUITE_END();

	void EnqueueTest();
	void DequeueTest();

    void ReadBlockTest();
    void WriteBlockTest();

    void CreateReaderTest();
    void CreateWriterTest();
    void CreateQueueTest();
    void CreateNodeTest();
    void ReaderIDTest();
    void WriterIDTest();
    void KernelIDTest();

    void Transfer(CPN::PacketEncoder *encoder, CPN::PacketDecoder *decoder);
};
#endif
