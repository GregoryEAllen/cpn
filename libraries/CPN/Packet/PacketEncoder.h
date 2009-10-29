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

#include "PacketHeader.h"
#include "AutoCircleBuffer.h"
#include <string>

namespace CPN {
    // Take the various message types that can be sent over the wire
    // and encode them into byte representation.
    class PacketEncoder {
    public:
        PacketEncoder();
        bool BytesReady() const;

        const void *GetEncodedBytes(unsigned &amount);
        void ReleaseEncodedBytes(unsigned amount);
        void Reset() { cbuff.Reset(); }
        unsigned NumBytes() const { return cbuff.Size(); }

        void SendEnqueue(const void **data, unsigned length, unsigned numchannels);
        void SendDequeue(unsigned length, unsigned numchannels);
        void SendReadBlock(unsigned requested);
        void SendWriteBlock(unsigned requested);
        void SendEndOfWriteQueue();
        void SendEndOfReadQueue();

        void SendReaderID(uint64_t readerkey, uint64_t writerkey);
        void SendWriterID(uint64_t writerkey, uint64_t readerkey);
    private:
        AutoCircleBuffer cbuff;
    };
}
#endif
