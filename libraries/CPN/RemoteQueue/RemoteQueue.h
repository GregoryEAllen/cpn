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
#ifndef CPN_REMOTEQUEUE_H
#define CPN_REMOTEQUEUE_H
#pragma once
#include "CPNCommon.h"
#include "ThresholdQueue.h"
#include "Pthread.h"
#include "PacketDecoder.h"
#include "PacketEncoder.h"
#include "LogicalClock.h"
#include "ConnectionServer.h"
#include "SocketHandle.h"
#include "D4RNode.h"
#include "Assert.h"
namespace CPN {
    class RemoteQueue
        : public ThresholdQueue,
        private Pthread,
        private PacketEncoder,
        private PacketDecoder
    {
    public:

        enum Mode_t {
            READ,
            WRITE
        };

        RemoteQueue(shared_ptr<Database> db, Mode_t mode,
                ConnectionServer *s, const SimpleQueueAttr &attr);

        Mode_t GetMode() const { return mode; }


        unsigned Count() const;
        bool Empty() const;
		unsigned Freespace() const;
		bool Full() const;
        unsigned QueueLength() const;
        void Grow(unsigned queueLen, unsigned maxThresh);
        void ShutdownReader();
        void ShutdownWriter();
    private:
        void WaitForData();
        void WaitForFreespace();
        void InternalDequeue(unsigned count);
        void InternalEnqueue(unsigned count);
        void SignalReaderTagChanged();
        void SignalWriterTagChanged();

        void SetupPacket(Packet &packet);
        void EnqueuePacket(const Packet &packet);
        void SendEnqueuePacket();
        void DequeuePacket(const Packet &packet);
        void SendDequeuePacket();
        void ReadBlockPacket(const Packet &packet);
        void SendReadBlockPacket();
        void WriteBlockPacket(const Packet &packet);
        void SendWriteBlockPacket();
        void EndOfWritePacket(const Packet &packet);
        void SendEndOfWritePacket();
        void EndOfReadPacket(const Packet &packet);
        void SendEndOfReadPacket();
        void GrowPacket(const Packet &packet);
        void SendGrowPacket();
        void D4RTagPacket(const Packet &packet);
        void SendD4RTagPacket();
        void IDReaderPacket(const Packet &packet);
        void IDWriterPacket(const Packet &packet);

        void Read();
        void WriteBytes(const iovec *iov, unsigned iovcnt);

        void *EntryPoint();
        void InternalCheckStatus();

        static unsigned QueueLength(unsigned length, double alpha, Mode_t mode);

        class MockNode : public D4R::Node {
        public:
            MockNode(Key_t key) : D4R::Node(key) {}
            void SignalTagChanged() { ASSERT(false); }
        };

        const Mode_t mode;
        const double alpha;
        ConnectionServer *const server;
        SocketHandle sock;
        MockNode mocknode;
        LogicalClock clock;

        unsigned readerlength;
        unsigned writerlength;

        /**
         * When in write mode this is the number of bytes that we think
         * are in the queue on the other side. When in read mode this is
         * the number of bytes that we have read from the socket minus
         * the number of bytes that have been dequeued.
         */
        unsigned bytecount;

        bool pendingBlock;
        bool sentEnd;
        bool pendingGrow;
        bool pendingD4RTag;

        bool incheckstatus;
    };
}
#endif
