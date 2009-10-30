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
#include "QueueBase.h"
#include "Assert.h"
#include <vector>

namespace CPN {

    PacketEncoder::PacketEncoder() {}

    PacketEncoder::~PacketEncoder() {}

    void PacketEncoder::SendEnqueue(const Packet &packet, QueueBase *queue) {
        ASSERT(packet.DataLength() == packet.NumChannels() * packet.Count());
        ASSERT(packet.Type() == PACKET_ENQUEUE);
        std::vector<iovec> iovs;
        // Must use const_cast here because iovec isn't const... :(
        iovec header = { const_cast<PacketHeader*>(&packet.header), sizeof(packet.header) };
        iovs.push_back(header);
        for (unsigned i = 0; i < packet.NumChannels(); ++i) {
            iovec iov;
            iov.iov_base = const_cast<void*>(queue->GetRawDequeuePtr(packet.Count(), i));
            ASSERT(iov.iov_base);
            iov.iov_len = packet.Count();
            iovs.push_back(iov);
        }
        WriteBytes(&iovs[0], iovs.size());
        queue->Dequeue(packet.Count());
    }

    void PacketEncoder::SendPacket(const Packet &packet) {
        iovec iov;
        iov.iov_base = const_cast<PacketHeader*>(&packet.header);
        iov.iov_len = sizeof(packet.header);
        WriteBytes(&iov, 1);
    }

    void PacketEncoder::SendPacket(const Packet &packet, void *data) {
        ASSERT(data);
        iovec iov[2];
        iov[0].iov_base = const_cast<PacketHeader*>(&packet.header);
        iov[0].iov_len = sizeof(packet.header);
        iov[1].iov_base = data;
        iov[1].iov_len = packet.DataLength();
        WriteBytes(&iov[0], 2);
    }

    BufferedPacketEncoder::BufferedPacketEncoder() {}

    bool BufferedPacketEncoder::BytesReady() const {
        return cbuff.Size() > 0;
    }

    const void *BufferedPacketEncoder::GetEncodedBytes(unsigned &amount) {
        return cbuff.AllocateGet(cbuff.Size(), amount);
    }

    void BufferedPacketEncoder::ReleaseEncodedBytes(unsigned amount) {
        cbuff.ReleaseGet(amount);
    }

    void BufferedPacketEncoder::WriteBytes(const iovec *iov, unsigned iovcnt) {
        for (unsigned i = 0; i < iovcnt; ++i) {
            cbuff.Put(static_cast<char*>(iov[i].iov_base), iov[i].iov_len);
        }
    }
}

