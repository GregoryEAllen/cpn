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

#include "PacketHeader.h"
#include "Assert.h"

namespace CPN {

    PacketHandler::~PacketHandler() {}

    void PacketHandler::FirePacket(const Packet &packet) {
        ASSERT(packet.Valid(), "Invalid packet");
        switch (packet.Type()) {
        case PACKET_ENQUEUE:
            EnqueuePacket(packet);
            break;
        case PACKET_DEQUEUE:
            DequeuePacket(packet);
            break;
        case PACKET_READBLOCK:
            ReadBlockPacket(packet);
            break;
        case PACKET_WRITEBLOCK:
            WriteBlockPacket(packet);
            break;
        case PACKET_ENDOFWRITE:
            EndOfWritePacket(packet);
            break;
        case PACKET_ENDOFREAD:
            EndOfReadPacket(packet);
            break;
        case PACKET_GROW:
            GrowPacket(packet);
            break;
        case PACKET_ID_READER:
            IDReaderPacket(packet);
            break;
        case PACKET_ID_WRITER:
            IDWriterPacket(packet);
            break;
        default:
            ASSERT(false, "Invalid packet type.");
        }
    }
}

