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
#include <string.h>

namespace CPN {

    PacketDecoder::PacketDecoder()
        : numbytes(0)
    {
    }

    PacketDecoder::~PacketDecoder() {}

    void *PacketDecoder::GetDecoderBytes(unsigned &amount) {
        char *ptr = static_cast<char*>(&header.header);
        amount = sizeof(header.header) - numbytes;
        return ptr + numbytes;
    }

    void PacketDecoder::ReleaseDecoderBytes(unsigned amount) {
        numbytes += amount;
        ASSERT(numbytes <= sizeof(header.header));
        if (numbytes == sizeof(header.header)) {
            if (header.Valid()) {
                FirePacket(header);
                numbytes = 0;
            } else {
                // If it is not valid, what do we do??
                // Well, we can search for the header word
                // wont always work...
                char *ptr = static_cast<char*>(&header.header);
                numbytes -= 1;
                memmove(ptr, ptr + 1, numbytes);
            }
        }
    }

    void PacketDecoder::Reset() {
        numbytes = 0;
    }
}

