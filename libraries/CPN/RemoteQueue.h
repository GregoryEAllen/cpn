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
#include "PacketDecoder.h"
#include "PacketEncoder.h"
#include "SocketHandle.h"

/*
 * Forward declarations.
 */
class ErrnoException;
class Pthread;
namespace D4R {
    class Node;
}

namespace CPN {
    class RemoteQueueHolder;

    /**
     * The RemoteQueue is a specialization of the ThresholdQueue which is split in half
     * across a socket. This class works closely with ConnectionServer and RemoteQueueHolder.
     */
    class RemoteQueue
        : public ThresholdQueue,
        private PacketEncoder,
        private PacketDecoder
    {
    public:

        enum Mode_t {
            READ,
            WRITE
        };

        RemoteQueue(shared_ptr<Database> db, Mode_t mode,
                ConnectionServer *s, RemoteQueueHolder *h, const SimpleQueueAttr &attr);

        ~RemoteQueue();

        /**
         * Start the internals of this queue.
         */
        void Start();

        Mode_t GetMode() const { return mode; }
        Key_t GetKey() const { return mode == READ ? readerkey : writerkey; }
        /**
         * Tell the queue that CPN is shutting down and the queue
         * should start cleaning up.
         */
        void Shutdown();

        unsigned Count() const;
        bool Empty() const;
        unsigned QueueLength() const;
        void Grow(unsigned queueLen, unsigned maxThresh);

        /// For debug ONLY!
        void LogState();
    private:
        void Signal();
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

        void *FileThreadEntryPoint();
        void *ActionThreadEntryPoint();
        void InternalCheckStatus();
        void UpdateClock(const Packet &packet);
        void TickClock();
        std::string ClockString() const;

        void HandleError(const ErrnoException &e);
        static unsigned QueueLength(unsigned length, unsigned maxthresh, double alpha, Mode_t mode);

        /// For debugging.
        std::string GetState();

        auto_ptr<Pthread> fileThread;
        auto_ptr<Pthread> actionThread;
        bool pendingAction;

        const Mode_t mode;
        const double alpha;
        ConnectionServer *const server;
        RemoteQueueHolder *const holder;
        SocketHandle sock;
        shared_ptr<D4R::Node> mocknode;
        uint64_t clock; // Our clock value
        uint64_t readclock; // Last knowledge of the reader clock
        uint64_t writeclock; // Last knowledge of the writer clock

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
        bool tagUpdated;

        bool dead;
    };
}
#endif
