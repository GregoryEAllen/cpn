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
            KernelMessageHandler *kernMsgHan,
            Key_t rkey, Key_t wkey)
        : kmh(kernMsgHan),
        readerkey(rkey),
        writerkey(wkey),
        endpoint(kernMsgHan, wkey, rkey, StreamEndpoint::READ)
    {
    }

    void ReaderStream::RegisterDescriptor(std::vector<Async::DescriptorPtr> &descriptors) {
        Async::DescriptorPtr desc = endpoint.GetDescriptor();
        if (desc) {
            descriptors.push_back(endpoint.GetDescriptor());
        } else if (endpoint.Shuttingdown()) {
            kmh->StreamDead(readerkey);
        }
        // Writer will be setup to create a new connection.
    }

    void ReaderStream::SetDescriptor(Async::DescriptorPtr desc) {
        endpoint.SetDescriptor(desc);
    }

    void ReaderStream::SetQueue(shared_ptr<QueueBase> q) {
        endpoint.SetQueue(q);
    }

}

