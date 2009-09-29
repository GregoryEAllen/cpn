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

    void PacketEncoder::SendEnqueue(void *data, unsigned length, unsigned numchannels) {
    }

    void PacketEncoder::SendDequeue(unsigned length, unsigned numchannels) {
    }

    void PacketEncoder::SendReadBlock(unsigned requested) {
    }

    void PacketEncoder::SendWriteBlock(unsigned requested) {
    }

    void PacketEncoder::SendCreateReader(
            unsigned queuehint, unsigned queuelenght, unsigned maxthreshold,
            unsigned numchannels, uint64_t readerkey, uint64_t writerkey)
    {
    }
    void PacketEncoder::SendCreateWriter(
            unsigned queuehint, unsigned queuelenght, unsigned maxthreshold,
            unsigned numchannels, uint64_t readerkey, uint64_t writerkey)
    {
    }
    void PacketEncoder::SendCreateQueue(
            unsigned queuehint, unsigned queuelenght, unsigned maxthreshold,
            unsigned numchannels, uint64_t readerkey, uint64_t writerkey)
    {
    }
    void PacketEncoder::SendCreateNode(
            const std::string &nodename,
            const std::string &nodetype,
            const std::string &param,
            const StaticConstBuffer arg,
            uint64_t nodekey,
            uint64_t hostkey)
    {
    }

    void PacketEncoder::SendReaderID(uint64_t readerkey) {
    }

    void PacketEncoder::SendWriterID(uint64_t writerkey) {
    }

    void PacketEncoder::SendKernelID(uint64_t kernelkey) {
    }

}

