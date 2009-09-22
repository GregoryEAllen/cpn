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

#include "WriterStream.h"
#include "QueueBase.h"

namespace CPN {

    WriterStream::WriterStream(
            Async::DescriptorPtr wu,
            shared_ptr<QueueBase> q,
            Key_t key)
        : wakeup(wu),
        queue(q),
        upstream(q->UpStreamChain()),
        writerkey(key),
        endpoint(q, q->DownStreamChain())
    {
        downstream = MsgQueueSignal<NodeMessagePtr, MsgQueue<NodeMessagePtr> >::Create();
        downstream->Connect(sigc::mem_fun(this, &WriterStream::MessageNotice));
        queue->DownStreamChain()->Chain(downstream);
    }

    void WriterStream::RegisterDescriptor(std::vector<Async::DescriptorPtr> &descriptors) {
        Async::DescriptorPtr desc = endpoint.GetDescriptor();
        if (desc) {
            descriptors.push_back(endpoint.GetDescriptor());
        } else {
            ASSERT(false, "Need connection setup");
        }
    }

    void WriterStream::RunOneIteration() {
        while (!downstream->Empty()) {
            NodeMessagePtr msg = downstream->Get();
            msg->DispatchOn(&endpoint);
        }
    }

    void WriterStream::SetDescriptor(Async::DescriptorPtr desc) {
        endpoint.SetDescriptor(desc);
    }

    void WriterStream::SetQueue(shared_ptr<QueueBase> q) {
    }

    void WriterStream::MessageNotice() {
        Async::Stream stream(wakeup);
        char c = 0;
        ENSURE(stream.Write(&c, sizeof(c)) == sizeof(c),
                "The write for notice from WriterStream was not atomic");
    }

    void WriterStream::OnError(int err) {
        // TODO actual error handling, abort for now
        ASSERT(false, "Todo actual error handling");
    }

}


