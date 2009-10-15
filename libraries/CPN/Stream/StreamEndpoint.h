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
#include "CPNStream.h"

#include "Message.h"

#include "PacketDecoder.h"
#include "PacketEncoder.h"

#include "ReentrantLock.h"

#include "AsyncStream.h"

namespace CPN {

    /**
     * This class does the actual translation between messages and packets.
     * The nature of this class depends upon the messages it receives.
     * If it receives dequeue messages then it acts like a reader if it is
     * receiving enqueue messages then it acts like a writer.
     */
    class StreamEndpoint : public ReaderMessageHandler,
                           public WriterMessageHandler,
                           private PacketDecoder,
                           public Stream
    {
    public:
        enum Mode_t { READ, WRITE };

        StreamEndpoint(KernelMessageHandler *kernMsgHan, Key_t rkey, Key_t wkey, Mode_t m);

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

        void OnError(int err);

        void RegisterDescriptor(std::vector<Async::DescriptorPtr> &descriptors);
        Async::DescriptorPtr GetDescriptor() { return descriptor; }
        void SetDescriptor(Async::DescriptorPtr desc);
        void ResetDescriptor();

        void SetQueue(shared_ptr<QueueBase> q);

        bool Shuttingdown() const;

        Mode_t GetMode() const { return mode; }
        Key_t ReaderKey() const { return readerkey; }
        Key_t WriterKey() const { return writerkey; }
        Key_t GetKey() const;

        void PrintState();
    private:
        void ReceivedEnqueue(void *data, unsigned length, unsigned numchannels);
        void ReceivedDequeue(unsigned length, unsigned numchannels);
        void ReceivedReadBlock(unsigned requested);
        void ReceivedWriteBlock(unsigned requested);
        void ReceiveEndOfWriteQueue();
        void ReceiveEndOfReadQueue();
        void ReceivedReaderID(uint64_t rkey, uint64_t wkey);
        void ReceivedWriterID(uint64_t wkey, uint64_t rkey);

        void CheckBlockedEnqueues();
        bool WriteEnqueue();
        bool EnqueueBlocked();
        void SignalDeath();
        void Wakeup();

        // If both locks are aquired, aquire the
        // queuelock first because we may already have it
        const Sync::ReentrantLock *queuelock;
        const Sync::ReentrantLock lock;
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
        KernelMessageHandler *kmh;

        const Key_t readerkey;
        const Key_t writerkey;
        const Mode_t mode;
        bool shuttingdown;
        bool readdead;
        bool writedead;
        weak_ptr<UnknownStream> pendingconn;
    };
}

#endif

