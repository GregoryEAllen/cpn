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
 * \brief The UnknownStream class handles a stream descriptor until it is known
 * what that descriptor represents.
 * \author John Bridgman
 */
#ifndef CPN_UNKNOWNSTREAM_H
#define CPN_UNKNOWNSTREAM_H
#pragma once
#include "CPNCommon.h"
#include "Message.h"
#include "PacketDecoder.h"
#include "AsyncSocket.h"
#include "IntrusiveRing.h"
namespace CPN {
    class UnknownStream
        : public IntrusiveRingElement<UnknownStream>,
        public PacketDecoder
   {
    public:
        UnknownStream(Async::SockPtr desc, KernelMessageHandler *kernMsgHan);

    private:

        void ReceivedReaderID(uint64_t readerkey, uint64_t writerkey);
        void ReceivedWriterID(uint64_t writerkey, uint64_t readerkey);
        void ReceivedKernelID(uint64_t srckernelkey, uint64_t dstkernelkey);

        bool ReadReady();
        void ReadSome();
        void DestroyThis();
        Async::DescriptorPtr descriptor;
        sigc::connection readready;
        sigc::connection readsome;
        KernelMessageHandler *kmh;
    };
}
#endif
