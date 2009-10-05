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
 * \brief WriterStream encapsulates the descriptor for
 * the stream communication on the writer end for a 
 * remote queue.
 * \author John Bridgman
 */

#ifndef CPN_WRITERSTREAM_H
#define CPN_WRITERSTREAM_H
#pragma once

#include "CPNCommon.h"
#include "StreamEndpoint.h"
#include "CPNStream.h"

#include "Message.h"

#include "AsyncStream.h"
#include <vector>

namespace CPN {

    class WriterStream : public Stream {
    public:

        WriterStream(
                KernelMessageHandler *kernMsgHan,
                Key_t wkey, Key_t rkey);

        void RegisterDescriptor(std::vector<Async::DescriptorPtr> &descriptors);

        void RunOneIteration();

        void SetDescriptor(Async::DescriptorPtr desc);

        void SetQueue(shared_ptr<QueueBase> q);

        Key_t GetKey() const { return writerkey; }
        Key_t GetReaderKey() const { return readerkey; }
    private:
        void MessageNotice();
        void OnError(int err);

        KernelMessageHandler *kmh;
        Key_t writerkey;
        Key_t readerkey;
        StreamEndpoint endpoint;
    };
}

#endif

