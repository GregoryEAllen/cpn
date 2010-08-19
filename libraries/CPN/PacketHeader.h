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
 * \brief Declarations of a generic binary packet format.
 *
 * Note that the order of the fields in these structs is deliberate in most
 * cases. They are ordered mostly to ensure that the compiler will pack the
 * data correctly into the 64 byte header.
 *
 * \author John Bridgman
 */

#ifndef PACKET_H
#define PACKET_H
#pragma once

#include <stdint.h>
#include <cstring>

namespace CPN {

    // All the packet headers are PODs so they can be unioned.

    /** \brief some packet header constants.
     */
    enum {
        PACKET_SYNCWORD = 0xF1F0C0DE,
        PACKET_REVERSE_SYNCWORD  = 0xDEC0F0F1,
        PACKET_HEADERLENGTH = 64
    };

    enum PacketType_t {
        PACKET_ENQUEUE,
        PACKET_DEQUEUE,
        PACKET_READBLOCK,
        PACKET_WRITEBLOCK,
        PACKET_ENDOFWRITE,
        PACKET_ENDOFREAD,
        PACKET_GROW,
        PACKET_D4RTAG,

        PACKET_ID_READER,
        PACKET_ID_WRITER
    };

    struct PacketHeader {
        union {
            struct {
                uint32_t syncWord;
                uint32_t dataLength;
                uint32_t dataType;
                union {
                    uint32_t bytesQueued;
                    uint32_t queueSize;
                };
                uint64_t srckey;
                uint64_t dstkey;
                union {
                    uint32_t requested;
                    uint32_t maxThresh;
                    uint32_t count;
                };
                uint32_t numChans;
                uint64_t clock;
                uint8_t mode;
                uint8_t status;
            };
            uint8_t pad[PACKET_HEADERLENGTH];
        };
    };

    inline bool ValidPacket(const PacketHeader *ph) {
        return ph->syncWord == PACKET_SYNCWORD;
    }

    inline void InitPacket(PacketHeader *ph, uint32_t datalen, PacketType_t type) {
        memset(ph, 0, sizeof(PacketHeader));
        ph->syncWord = PACKET_SYNCWORD;
        ph->dataLength = datalen;
        ph->dataType = type;
    }

    class Packet {
    public:
        Packet() { memset(&header, 0, sizeof(header)); }
        Packet(uint32_t datalen, PacketType_t type) { Init(datalen, type); }
        Packet(PacketType_t type) { Init(0, type); }
        Packet(const PacketHeader &ph) : header(ph) {}

        Packet &Init(uint32_t datalen, PacketType_t type) {
            InitPacket(&header, datalen, type);
            return *this;
        }

        uint32_t DataLength() const { return header.dataLength; }
        PacketType_t Type() const { return static_cast<PacketType_t>(header.dataType); }
        uint32_t BytesQueued() const { return header.bytesQueued; }
        uint32_t QueueSize() const { return header.queueSize; }
        uint64_t SourceKey() const { return header.srckey; }
        uint64_t DestinationKey() const { return header.dstkey; }
        uint32_t Requested() const { return header.requested; }
        uint32_t MaxThreshold() const { return header.maxThresh; }
        uint32_t Count() const { return header.count; }
        uint32_t NumChannels() const { return header.numChans; }
        uint64_t Clock() const { return header.clock; }
        uint8_t Mode() const { return header.mode; }
        uint8_t Status() const { return header.status; }
        bool Valid() const { return ValidPacket(&header); }

        Packet &DataLength(uint32_t dl) { header.dataLength = dl; return *this; }
        Packet &Type(PacketType_t t) { header.dataType = t; return *this; }
        Packet &BytesQueued(uint32_t bq) { header.bytesQueued = bq; return *this; }
        Packet &QueueSize(uint32_t qs) { header.queueSize = qs; return *this; }
        Packet &SourceKey(uint64_t k) { header.srckey = k; return *this; }
        Packet &DestinationKey(uint64_t k) { header.dstkey = k; return *this; }
        Packet &Requested(uint32_t r) { header.requested = r; return *this; }
        Packet &MaxThreshold(uint32_t mt) { header.maxThresh = mt; return *this; }
        Packet &Count(uint32_t cnt) { header.count = cnt; return *this; }
        Packet &NumChannels(uint32_t nc) { header.numChans = nc; return *this; }
        Packet &Clock(uint64_t c) { header.clock = c; return *this; }
        Packet &Mode(uint8_t m) { header.mode = m; return *this; }
        Packet &Status(uint8_t s) { header.status = s; return *this; }

    public:
        PacketHeader header;
    };

    class PacketHandler {
    public:
        virtual ~PacketHandler();
        void FirePacket(const Packet &packet);

        virtual void EnqueuePacket(const Packet &packet) = 0;
        virtual void DequeuePacket(const Packet &packet) = 0;
        virtual void ReadBlockPacket(const Packet &packet) = 0;
        virtual void WriteBlockPacket(const Packet &packet) = 0;
        virtual void EndOfWritePacket(const Packet &packet) = 0;
        virtual void EndOfReadPacket(const Packet &packet) = 0;
        virtual void GrowPacket(const Packet &packet) = 0;
        virtual void D4RTagPacket(const Packet &packet) = 0;
        virtual void IDReaderPacket(const Packet &packet) = 0;
        virtual void IDWriterPacket(const Packet &packet) = 0;
    };
}

#endif

