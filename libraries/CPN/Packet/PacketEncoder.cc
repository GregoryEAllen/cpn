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

#include "PacketEncoder.h"

namespace CPN {

    PacketEncoder::PacketEncoder() {
    }

    bool PacketEncoder::BytesReady() {
        return cbuff.Size() > 0;
    }

    void *PacketEncoder::GetEncodedBytes(unsigned &amount) {
        return cbuff.AllocateGet(cbuff.Size(), amount);
    }

    void PacketEncoder::ReleaseEncodedBytes(unsigned amount) {
        cbuff.ReleaseGet(amount);
    }

    void PacketEncoder::SendEnqueue(void **data, unsigned length, unsigned numchannels) {
        PacketHeader header;
        InitPacket(&header, length * numchannels, PACKET_ENQUEUE);
        header.enqueue.amount = length;
        header.enqueue.numchannels = numchannels;
        cbuff.Put((char*)&header, sizeof(header));
        for (unsigned i = 0; i < numchannels; ++i) {
            cbuff.Put((char*)data[i], length);
        }
    }

    void PacketEncoder::SendDequeue(unsigned length, unsigned numchannels) {
        PacketHeader header;
        InitPacket(&header, 0, PACKET_DEQUEUE);
        header.dequeue.amount = length;
        header.dequeue.numchannels = numchannels;
        cbuff.Put((char*)&header, sizeof(header));
    }

    void PacketEncoder::SendReadBlock(unsigned requested) {
        PacketHeader header;
        InitPacket(&header, 0, PACKET_READBLOCK);
        header.readblock.requested = requested;
        cbuff.Put((char*)&header, sizeof(header));
    }

    void PacketEncoder::SendWriteBlock(unsigned requested) {
        PacketHeader header;
        InitPacket(&header, 0, PACKET_WRITEBLOCK);
        header.writeblock.requested = requested;
        cbuff.Put((char*)&header, sizeof(header));
    }

    void PacketEncoder::SendCreateReader(
            unsigned queuehint, unsigned queuelength, unsigned maxthreshold,
            unsigned numchannels, uint64_t readerkey, uint64_t writerkey)
    {
        PacketHeader header;
        InitPacket(&header, 0, PACKET_CREATE_READER);
        header.createqueue.queuehint = queuehint;
        header.createqueue.queuelength = queuelength;
        header.createqueue.maxthreshold = maxthreshold;
        header.createqueue.numchannels = numchannels;
        header.createqueue.readerkey = readerkey;
        header.createqueue.writerkey = writerkey;
        cbuff.Put((char*)&header, sizeof(header));
    }
    void PacketEncoder::SendCreateWriter(
            unsigned queuehint, unsigned queuelength, unsigned maxthreshold,
            unsigned numchannels, uint64_t readerkey, uint64_t writerkey)
    {
        PacketHeader header;
        InitPacket(&header, 0, PACKET_CREATE_WRITER);
        header.createqueue.queuehint = queuehint;
        header.createqueue.queuelength = queuelength;
        header.createqueue.maxthreshold = maxthreshold;
        header.createqueue.numchannels = numchannels;
        header.createqueue.readerkey = readerkey;
        header.createqueue.writerkey = writerkey;
        cbuff.Put((char*)&header, sizeof(header));
    }
    void PacketEncoder::SendCreateQueue(
            unsigned queuehint, unsigned queuelength, unsigned maxthreshold,
            unsigned numchannels, uint64_t readerkey, uint64_t writerkey)
    {
        PacketHeader header;
        InitPacket(&header, 0, PACKET_CREATE_QUEUE);
        header.createqueue.queuehint = queuehint;
        header.createqueue.queuelength = queuelength;
        header.createqueue.maxthreshold = maxthreshold;
        header.createqueue.numchannels = numchannels;
        header.createqueue.readerkey = readerkey;
        header.createqueue.writerkey = writerkey;
        cbuff.Put((char*)&header, sizeof(header));
    }
    void PacketEncoder::SendCreateNode(
            const std::string &nodename,
            const std::string &nodetype,
            const std::string &param,
            const StaticConstBuffer arg,
            uint64_t nodekey,
            uint64_t hostkey)
    {
        PacketHeader header;
        InitPacket(&header, 0, PACKET_CREATE_NODE);
        unsigned totalLen = 0;
        totalLen += header.createnode.namelen = nodename.length();
        totalLen += header.createnode.typelen = nodetype.length();
        totalLen += header.createnode.paramlen = param.length();
        totalLen += header.createnode.arglen = arg.GetSize();
        header.createnode.nodekey = nodekey;
        header.createnode.hostkey = hostkey;
        header.base.dataLength = totalLen;

        cbuff.Put((char*)&header, sizeof(header));
        cbuff.Put(nodename.data(), nodename.length());
        cbuff.Put(nodetype.data(), nodetype.length());
        cbuff.Put(param.data(), param.length());
        cbuff.Put(arg);
    }

    void PacketEncoder::SendReaderID(uint64_t readerkey) {
        PacketHeader header;
        InitPacket(&header, 0, PACKET_ID_READER);
        header.identify.key = readerkey;
        cbuff.Put((char*)&header, sizeof(header));
    }

    void PacketEncoder::SendWriterID(uint64_t writerkey) {
        PacketHeader header;
        InitPacket(&header, 0, PACKET_ID_WRITER);
        header.identify.key = writerkey;
        cbuff.Put((char*)&header, sizeof(header));
    }

    void PacketEncoder::SendKernelID(uint64_t kernelkey) {
        PacketHeader header;
        InitPacket(&header, 0, PACKET_ID_KERNEL);
        header.identify.key = kernelkey;
        cbuff.Put((char*)&header, sizeof(header));
    }

}

