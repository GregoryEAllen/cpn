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

        PACKET_CREATE_READER,
        PACKET_CREATE_WRITER,
        PACKET_CREATE_QUEUE,
        PACKET_CREATE_NODE,

        PACKET_ID_READER,
        PACKET_ID_WRITER,
        PACKET_ID_KERNEL
    };

    struct PacketHeaderBase {
        uint32_t syncWord;
        uint32_t dataLength;
        uint32_t dataType;
    };

    struct EnqueuePacketHeader {
        PacketHeaderBase base;
        uint32_t amount;
        uint32_t numchannels;
    };

    struct DequeuePacketHeader {
        PacketHeaderBase base;
        uint32_t amount;
        uint32_t numchannels;
    };

    struct ReadBlockPacketHeader {
        PacketHeaderBase base;
        uint32_t requested;
    };

    struct WriteBlockPacketHeader {
        PacketHeaderBase base;
        uint32_t requested;
    };

    struct CreateQueuePacketHeader {
        PacketHeaderBase base;
        uint32_t queuehint;
        uint32_t queuelength;
        uint32_t maxthreshold;
        uint32_t numchannels;

        uint32_t packet_header_field_alignment;

        uint64_t readerkey;
        uint64_t writerkey;
    };

    struct CreateNodePacketHeader {
        PacketHeaderBase base;
        // The parameters name, nodetype, param, and arg
        // will we placed in the data section in that order
        // These four fields give the length of each of those sections.
        // To compute the offset just add up the length.
        uint32_t namelen;
        uint32_t typelen;
        uint32_t paramlen;
        uint32_t arglen;

        uint32_t packet_header_field_alignment;

        uint64_t nodekey;
        uint64_t hostkey;
    };

    struct IdentifyPacketHeader {
        PacketHeaderBase base;
        uint64_t key;
    };

    struct PacketHeader {
        union {
            PacketHeaderBase base;
            EnqueuePacketHeader enqueue;
            DequeuePacketHeader dequeue;
            ReadBlockPacketHeader readblock;
            WriteBlockPacketHeader writeblock;
            CreateQueuePacketHeader createqueue;
            CreateNodePacketHeader createnode;
            IdentifyPacketHeader identify;
            uint8_t pad[PACKET_HEADERLENGTH];
        };
        char data[0];
    };

    inline bool ValidPacket(PacketHeader *ph) {
        return ph->base.syncWord == PACKET_SYNCWORD;
    }

    inline void InitPacket(PacketHeader *ph, uint32_t datalen, PacketType_t type) {
        memset(ph, 0, sizeof(PacketHeader));
        ph->base.syncWord = PACKET_SYNCWORD;
        ph->base.dataLength = datalen;
        ph->base.dataType = type;
    }
}

#endif

