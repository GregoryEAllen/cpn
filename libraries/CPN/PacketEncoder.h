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
#ifndef CPN_PACKETENCODER_H
#define CPN_PAcKETENCODER_H
#pragma once

#include "CPNCommon.h"
#include "PacketHeader.h"
#include "CircularQueue.h"
#include "ThrowingAssert.h"
#include <vector>
// For the iovec definition
#include <sys/uio.h>

namespace CPN {
    /** Take the various message types that can be sent over the wire
     * and encode them into byte representation.
     */
    class CPN_LOCAL PacketEncoder {
    public:
        PacketEncoder();
        virtual ~PacketEncoder();

        template<typename Queue_t>
        void SendEnqueue(const Packet &packet, Queue_t &queue) {
            ASSERT(packet.Type() == PACKET_ENQUEUE);
            std::vector<iovec> iovs;
            // Must use const_cast here because iovec.iov_base isn't const... :(
            iovec header = { const_cast<PacketHeader*>(&packet.header), sizeof(packet.header) };
            iovs.push_back(header);
            for (unsigned i = 0; i < queue.NumChannels(); ++i) {
                iovec iov;
                iov.iov_base = const_cast<void*>(queue.GetRawDequeuePtr(packet.Count(), i));
                ASSERT(iov.iov_base);
                iov.iov_len = packet.Count();
                iovs.push_back(iov);
            }
            WriteBytes(&iovs[0], iovs.size());
            queue.Dequeue(packet.Count());
        }

        void SendPacket(const Packet &packet);
        void SendPacket(const Packet &packet, void *data);
    protected:
        virtual void WriteBytes(const iovec *iov, unsigned iovcnt) = 0;
    private:
    };

    class CPN_LOCAL BufferedPacketEncoder : public PacketEncoder {
    public:
        BufferedPacketEncoder();

        bool BytesReady() const;
        const void *GetEncodedBytes(unsigned &amount);
        void ReleaseEncodedBytes(unsigned amount);
        void Reset() { cbuff.Clear(); }
        unsigned NumBytes() const { return cbuff.Count(); }
    protected:
        virtual void WriteBytes(const iovec *iov, unsigned iovcnt);
    private:
        CircularQueue cbuff;
    };
}
#endif
