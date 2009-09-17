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

#include "common.h"
#include "StreamEndpoint.h"
#include "CPNStream.h"
#include "MessageQueue.h"
#include "NodeMessage.h"
#include "AsyncStream.h"
#include <vector>

namespace CPN {

    class WriterStream {
    public:

        WriterStream(
                Async::DescriptorPtr wu,
                shared_ptr<QueueBase> q,
                Key_t key);

        void RegisterDescriptor(std::vector<Async::DescriptorPtr> &descriptors);

        void RunOneIteration();

        Key_t GetKey() const { return writerkey; }
    private:
        void MessageNotice();
        void OnError(int err);

        Async::DescriptorPtr wakeup;
        shared_ptr<QueueBase> queue;
        shared_ptr<MsgPut<NodeMessagePtr> > upstream;
        shared_ptr<MsgQueueSignal<NodeMessagePtr, MsgQueue<NodeMessagePtr> > > downstream;
        Key_t writerkey;
        StreamEndpoint endpoint;
    };
}

#endif

