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
#ifndef CPN_PACKETDECODER_H
#define CPN_PACKETDECODER_H
#pragma once

#include "CPNCommon.h"
#include "PacketHeader.h"

namespace CPN {

    /**
     * The packet decoder receives bytes through GetDecoderbytes and
     * ReleaseDecoderBytes.  When enough bytes have been recieved to call one
     * of the Received event functions those events will be called and the
     * internal state will remove the information.  If the information is not
     * delt with in the event function it will be lost.  This class is ment to
     * be enherited by some other class which implements the event functions.
     */
    class PacketDecoder : public PacketHandler {
    public:
        PacketDecoder();
        virtual ~PacketDecoder();
        // amount is the amount the decoder wants for the next operation
        void *GetDecoderBytes(unsigned &amount);
        void ReleaseDecoderBytes(unsigned amount);
        void Reset();
        unsigned NumBytes() const { return numbytes; }

    protected:
    private:
        Packet header;
        unsigned numbytes;
    };

}
#endif
