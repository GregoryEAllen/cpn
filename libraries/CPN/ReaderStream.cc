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

#include "ReaderStream.h"
#include "QueueBase.h"

namespace CPN {

    ReaderStream::ReaderStream(
            Async::DescriptorPtr wu,
            shared_ptr<QueueBase> q,
            Key_t key)
        : wakeup(wu),
        queue(q),
        downstream(q->DownStreamChain()),
        readerkey(key),
        endpoint(q, q->DownStreamChain())
    {
        upstream = MsgQueueSignal<NodeMessagePtr, MsgQueue<NodeMessagePtr> >::Create();
        upstream->Connect(sigc::mem_fun(this, &ReaderStream::MessageNotice));
        queue->UpStreamChain()->Chain(upstream);
    }

    void ReaderStream::RegisterDescriptor(std::vector<Async::DescriptorPtr> &descriptors) {
        Async::DescriptorPtr desc = endpoint.GetDescriptor();
        if (desc) {
            descriptors.push_back(endpoint.GetDescriptor());
        }
        // Writer will be setup to create a new connection.
    }

    void ReaderStream::RunOneIteration() {
        while (!upstream->Empty()) {
            NodeMessagePtr msg = upstream->Get();
            msg->DispatchOn(&endpoint);
        }
    }

    void ReaderStream::SetDescriptor(Async::DescriptorPtr desc) {
        endpoint.SetDescriptor(desc);
    }

    void ReaderStream::SetQueue(shared_ptr<QueueBase> q) {
    }

    void ReaderStream::MessageNotice() {
        Async::Stream stream(wakeup);
        char c = 0;
        ENSURE(stream.Write(&c, sizeof(c)) == sizeof(c),
                "The write for notice from ReaderStream was not atomic");
    }

    void ReaderStream::OnError(int err) {
        // TODO actual error handling, abort for now
        ASSERT(false);
    }
}

