//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \author John Bridgman
 */

#include "PacketDecoder.h"
#include "Assert.h"

namespace CPN {

    PacketDecoder::PacketDecoder() : enable(true) {
        Reset();
    }

    void *PacketDecoder::GetDecoderBytes(unsigned &amount) {
        if (!enable) {
            amount = 0;
            return 0;
        }
        amount = packetsize - numbytes;
        return buffer.GetBuffer(numbytes);
    }

    void PacketDecoder::ReleaseDecoderBytes(unsigned amount) {
        ASSERT(enable);
        numbytes += amount;
        ASSERT(numbytes <= packetsize, "Tried to add more bytes than there are in the packet to"
                " the decoder.");
        if (numbytes == PACKET_HEADERLENGTH && packetsize == PACKET_HEADERLENGTH) {
            // Decode the header fire off the message
            // and reset if need be or reset the packetsize
            // and continue
            PacketHeader *header = (PacketHeader*)buffer.GetBuffer();
            ASSERT(ValidPacket(header), "Invalid packet header!");
            if (header->base.dataLength == 0) {
                Fire();
                Reset();
            } else {
                packetsize = PACKET_HEADERLENGTH + header->base.dataLength;
                buffer.EnsureSize(packetsize);
            }
        } else if (numbytes == packetsize) {
            // Fire off the message and reset the packetsize
            Fire();
            Reset();
        }
    }

    void PacketDecoder::Reset() {
        numbytes = 0;
        packetsize = PACKET_HEADERLENGTH;
        buffer.EnsureSize(packetsize);
    }

    void PacketDecoder::Fire() {
        PacketHeader *header = (PacketHeader*)buffer.GetBuffer();
        switch (header->base.dataType) {
        case PACKET_ENQUEUE:
            ReceivedEnqueue(buffer.GetBuffer(PACKET_HEADERLENGTH),
                    header->enqueue.amount, header->enqueue.numchannels);
            break;
        case PACKET_DEQUEUE:
            ReceivedDequeue(header->dequeue.amount, header->dequeue.numchannels);
            break;
        case PACKET_READBLOCK:
            ReceivedReadBlock(header->readblock.requested);
            break;
        case PACKET_WRITEBLOCK:
            ReceivedWriteBlock(header->writeblock.requested);
            break;
        case PACKET_ENDOFWRITEQUEUE:
            ReceiveEndOfWriteQueue();
            break;
        case PACKET_ENDOFREADQUEUE:
            ReceiveEndOfReadQueue();
            break;
        case PACKET_ID_READER:
            ReceivedReaderID(header->identify.srckey,
                    header->identify.dstkey);
            break;
        case PACKET_ID_WRITER:
            ReceivedWriterID(header->identify.srckey,
                    header->identify.dstkey);
            break;
        default:
            ASSERT(false, "Unknown packet type!");
        }
    }

    void PacketDecoder::ReceivedEnqueue(void *data, unsigned length,
            unsigned numchannels) {
        ASSERT(false, "Unexpected packet type received.");
    }

    void PacketDecoder::ReceivedDequeue(unsigned length, unsigned numchannels) {
        ASSERT(false, "Unexpected packet type received.");
    }

    void PacketDecoder::ReceivedReadBlock(unsigned requested) {
        ASSERT(false, "Unexpected packet type received.");
    }

    void PacketDecoder::ReceivedWriteBlock(unsigned requested) {
        ASSERT(false, "Unexpected packet type received.");
    }

    void PacketDecoder::ReceiveEndOfWriteQueue() {
        ASSERT(false, "Unexpected packet type received.");
    }

    void PacketDecoder::ReceiveEndOfReadQueue() {
        ASSERT(false, "Unexpected packet type received.");
    }

    void PacketDecoder::ReceivedReaderID(uint64_t readerkey, uint64_t writerkey) {
        ASSERT(false, "Unexpected packet type received.");
    }

    void PacketDecoder::ReceivedWriterID(uint64_t writerkey, uint64_t readerkey) {
        ASSERT(false, "Unexpected packet type received.");
    }

}

