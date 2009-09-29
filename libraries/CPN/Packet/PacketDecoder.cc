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

    PacketDecoder::PacketDecoder() {
        Reset();
    }

    void *PacketDecoder::GetDecoderBytes(unsigned &amount) {
        amount = packetsize - numbytes;
        return buffer.GetBuffer(numbytes);
    }

    void PacketDecoder::ReleaseDecoderBytes(unsigned amount) {
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
        case PACKET_CREATE_READER:
            ReceivedCreateReader(
                    header->createqueue.queuehint,
                    header->createqueue.queuelength,
                    header->createqueue.maxthreshold,
                    header->createqueue.numchannels,
                    header->createqueue.readerkey,
                    header->createqueue.writerkey
                    );
            break;
        case PACKET_CREATE_WRITER:
            ReceivedCreateWriter(
                    header->createqueue.queuehint,
                    header->createqueue.queuelength,
                    header->createqueue.maxthreshold,
                    header->createqueue.numchannels,
                    header->createqueue.readerkey,
                    header->createqueue.writerkey
                    );
            break;
        case PACKET_CREATE_QUEUE:
            ReceivedCreateQueue(
                    header->createqueue.queuehint,
                    header->createqueue.queuelength,
                    header->createqueue.maxthreshold,
                    header->createqueue.numchannels,
                    header->createqueue.readerkey,
                    header->createqueue.writerkey
                    );
            break;
        case PACKET_CREATE_NODE:
            {
                char *strbase = (char*)buffer.GetBuffer(PACKET_HEADERLENGTH);
                unsigned length = header->createnode.namelen;
                std::string nodename(strbase, length);
                strbase += length;
                length = header->createnode.typelen;
                std::string nodetype(strbase, length);
                strbase += length;
                length = header->createnode.paramlen;
                std::string param(strbase, length);
                strbase += length;
                length = header->createnode.arglen;
                ReceivedCreateNode(
                        nodename,
                        nodetype,
                        param,
                        StaticConstBuffer(strbase, length),
                        header->createnode.nodekey,
                        header->createnode.hostkey
                        );
            }
            break;
        case PACKET_ID_READER_STREAM:
            ReceivedReaderID(header->identify.key);
            break;
        case PACKET_ID_WRITER_STREAM:
            ReceivedWriterID(header->identify.key);
            break;
        case PACKET_ID_KERNEL_STREAM:
            ReceivedKernelID(header->identify.key);
            break;
        default:
            ASSERT(false, "Unknown packet type!");
        }
    }

    void PacketDecoder::ReceivedEnqueue(void *data, unsigned length,
            unsigned numchannels) {
    }

    void PacketDecoder::ReceivedDequeue(unsigned length, unsigned numchannels) {
    }

    void PacketDecoder::ReceivedReadBlock(unsigned requested) {
    }

    void PacketDecoder::ReceivedWriteBlock(unsigned requested) {
    }

    void PacketDecoder::ReceivedCreateReader(
            unsigned queuehint, unsigned queuelenght, unsigned maxthreshold,
            unsigned numchannels, uint64_t readerkey, uint64_t writerkey)
    {
    }

    void PacketDecoder::ReceivedCreateWriter(
            unsigned queuehint, unsigned queuelenght, unsigned maxthreshold,
            unsigned numchannels, uint64_t readerkey, uint64_t writerkey)
    {
    }

    void PacketDecoder::ReceivedCreateQueue(
            unsigned queuehint, unsigned queuelenght, unsigned maxthreshold,
            unsigned numchannels, uint64_t readerkey, uint64_t writerkey)
    {
    }

    void PacketDecoder::ReceivedCreateNode(
            const std::string &nodename,
            const std::string &nodetype,
            const std::string &param,
            const StaticConstBuffer arg,
            uint64_t nodekey,
            uint64_t hostkey)
    {
    }

    void PacketDecoder::ReceivedReaderID(uint64_t readerkey) {
    }

    void PacketDecoder::ReceivedWriterID(uint64_t writerkey) {
    }

    void PacketDecoder::ReceivedKernelID(uint64_t kernelkey) {
    }

}

