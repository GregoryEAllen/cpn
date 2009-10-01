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
 * \brief StreamEndpoint handles the actual translation
 * between the stream and the queue protocol.
 * \author John Bridgman
 */

#ifndef CPN_STREAMENDPOINT_H
#define CPN_STREAMENDPOINT_H
#pragma once

#include "common.h"
#include "NodeMessage.h"
#include "MessageQueue.h"

#include "PacketDecoder.h"
#include "PacketEncoder.h"


#include "AsyncStream.h"
#include "AutoCircleBuffer.h"
#include "Assert.h"

namespace CPN {

    /**
     * This class does the actual translation between messages and packets.
     * The nature of this class depends upon the messages it receives.
     * If it receives dequeue messages then it acts like a reader if it is
     * receiving enqueue messages then it acts like a writer.
     */
    class StreamEndpoint : public NodeMsgDispatch,
                           private PacketDecoder
    {
    public:

        StreamEndpoint();

        // From NodeMsgDispatch

        // A StreamEndpoint should not recieve these two messages.
        void ProcessMessage(NodeSetReader *msg) { ASSERT(false, "Invalid Message Type to StreamEndpoint"); }
        void ProcessMessage(NodeSetWriter *msg) { ASSERT(false, "Invalid Message Type to StreamEndpoint"); }

        // These messages get sent to the other side
        void ProcessMessage(NodeEnqueue *msg);
        void ProcessMessage(NodeDequeue *msg);
        void ProcessMessage(NodeReadBlock *msg);
        void ProcessMessage(NodeWriteBlock *msg);
        void Shutdown();
        void ProcessMessage(NodeEndOfWriteQueue *msg);
        void ProcessMessage(NodeEndOfReadQueue *msg);

        /**
         * \return true when we want to be notified 
         * when the descriptor is readable
         */
        bool ReadReady();
        /**
         * \return true when we want to be notified when
         * the descriptor is writeable
         */
        bool WriteReady();
        /**
         * Function called when the descriptor is readable.
         */
        void ReadSome();
        /**
         * Function called when the descriptor is writeable.
         */
        void WriteSome();

        Async::DescriptorPtr GetDescriptor() { return descriptor; }
        void SetDescriptor(Async::DescriptorPtr desc);

        void SetQueue(shared_ptr<QueueBase> q,
                shared_ptr<MsgPut<NodeMessagePtr> > mfs);

    private:
        void ReceivedEnqueue(void *data, unsigned length, unsigned numchannels);
        void ReceivedDequeue(unsigned length, unsigned numchannels);
        void ReceivedReadBlock(unsigned requested);
        void ReceivedWriteBlock(unsigned requested);

        void WriteEnqueue(NodeEnqueuePtr msg);

        void CheckBlockedEnqueues();
        bool Blocked();

        PacketEncoder encoder;

        Async::DescriptorPtr descriptor;
        // This is how many bytes this endpoint thinks it is in
        // the other endpoints queue. Unused in read mode.
        unsigned writecount;
        shared_ptr<QueueBase> queue;

        shared_ptr<MsgPut<NodeMessagePtr> > msgtoendpoint;
        std::deque<NodeEnqueuePtr> blockedenqueues;
    };
}

#endif

