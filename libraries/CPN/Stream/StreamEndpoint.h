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

#include "CPNCommon.h"

#include "Message.h"

#include "PacketDecoder.h"
#include "PacketEncoder.h"

#include "ReentrantLock.h"

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
    class StreamEndpoint : public ReaderMessageHandler,
                           public WriterMessageHandler,
                           private PacketDecoder
    {
    public:
        enum Mode_t { READ, WRITE };

        StreamEndpoint(Key_t rkey, Key_t wkey, Mode_t m);

        void RMHEnqueue(Key_t wkey, Key_t rkey);
        void RMHEndOfWriteQueue(Key_t wkey, Key_t rkey);
        void RMHWriteBlock(Key_t wkey, Key_t rkey, unsigned requested);
        void RMHTagChange(Key_t wkey, Key_t rkey);

        void WMHDequeue(Key_t rkey, Key_t wkey);
        void WMHEndOfReadQueue(Key_t rkey, Key_t wkey);
        void WMHReadBlock(Key_t rkey, Key_t wkey, unsigned requested);
        void WMHTagChange(Key_t rkey, Key_t wkey);

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
        void ResetDescriptor();

        void SetQueue(shared_ptr<QueueBase> q);

        bool Shuttingdown() const { return shuttingdown; }
    private:
        void ReceivedEnqueue(void *data, unsigned length, unsigned numchannels);
        void ReceivedDequeue(unsigned length, unsigned numchannels);
        void ReceivedReadBlock(unsigned requested);
        void ReceivedWriteBlock(unsigned requested);
        void ReceiveEndOfWriteQueue();
        void ReceiveEndOfReadQueue();

        void CheckBlockedEnqueues();
        bool WriteEnqueue();
        bool EnqueueBlocked();

        const Sync::ReentrantLock *lock;
        PacketEncoder encoder;

        Async::DescriptorPtr descriptor;
        // This is how many bytes this endpoint thinks it is in
        // the other endpoints queue. Unused in read mode.
        unsigned writecount;
        // Number of bytes we have placed in the queue and have
        // not seen a read for
        unsigned readcount;
        shared_ptr<QueueBase> queue;
        ReaderMessageHandler *rmh;
        WriterMessageHandler *wmh;

        Key_t readerkey;
        Key_t writerkey;
        Mode_t mode;
        bool shuttingdown;
    };
}

#endif

