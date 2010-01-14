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

#ifndef CPN_SOCKETENDPOINT_H
#define CPN_SOCKETENDPOINT_H
#pragma once

#include "CPNCommon.h"
#include "KernelBase.h"
#include "QueueBase.h"
#include "SockHandler.h"
#include "PacketHeader.h"
#include "CircularQueue.h"
#include "PacketDecoder.h"
#include "PacketEncoder.h"

namespace CPN {
    
    class SocketEndpoint 
        : public QueueBase, public SockHandler,
        private PacketEncoder, private PacketDecoder
    {
    public:

        enum Status_t {
            LIVE,
            DIEING,
            DEAD
        };

        enum Mode_t {
            READ,
            WRITE
        };

        SocketEndpoint(shared_ptr<Database> db, Mode_t mode,
                KernelBase *kmh_, const SimpleQueueAttr &attr);

        Status_t GetStatus() const;

        Mode_t GetMode() const { return mode; }
        Key_t GetKey() const { return mode == READ ? readerkey : writerkey; }

        /** 
         * to be removed
         */
        void Shutdown();

        /** Do any periodic things (like timing out a connection)
         * and/or return the maximum time we want to be checked again.
         */
        double CheckStatus();

        // QueueBase
    protected:
        virtual const void *InternalGetRawDequeuePtr(unsigned thresh, unsigned chan);
        virtual void InternalDequeue(unsigned count);
		virtual void *InternalGetRawEnqueuePtr(unsigned thresh, unsigned chan);
		virtual void InternalEnqueue(unsigned count);
    public:
        virtual unsigned NumChannels() const;
        virtual unsigned Count() const;
        virtual bool Empty() const;
		virtual unsigned Freespace() const;
		virtual bool Full() const;
        virtual unsigned MaxThreshold() const;
        virtual unsigned QueueLength() const;
        virtual void Grow(unsigned queueLen, unsigned maxThresh);

        // convenience function to print out state to the logger
        void LogState();
    protected:

        virtual void ShutdownReader();
        virtual void ShutdownWriter();
        virtual void WaitForData(unsigned request);
        virtual void WaitForFreespace(unsigned request);

        // FileHandler
        virtual void OnRead();
        virtual void OnWrite();
        virtual void OnError();
        virtual void OnHup();
        virtual void OnInval();

        virtual bool Readable() const;
    private:

        // PacketHandler (PacketDecoder)
        virtual void EnqueuePacket(const Packet &packet);
        virtual void DequeuePacket(const Packet &packet);
        virtual void ReadBlockPacket(const Packet &packet);
        virtual void WriteBlockPacket(const Packet &packet);
        virtual void EndOfWritePacket(const Packet &packet);
        virtual void EndOfReadPacket(const Packet &packet);
        virtual void IDReaderPacket(const Packet &packet);
        virtual void IDWriterPacket(const Packet &packet);

        // PacketEncoder
        virtual void WriteBytes(const iovec *iov, unsigned iovcnt);

        /** 
         * InternCheckStatus will do things like check the pending variables
         * and write data out if it thinks it should go out. It will also do things
         * like if there is a pending readblock then call OnRead and then check if
         * we received an enqueue.
         */
        void InternCheckStatus();
        bool EnqueueBlocked();

        void SetupPacketDefaults(Packet &packet);

        void SendEnqueue();
        void SendWriteBlock();
        void SendEndOfWrite();

        void SendDequeue();
        void SendReadBlock();
        void SendEndOfRead();


        Logger logger;
        ::CircularQueue queue;

        Status_t status;
        const Mode_t mode;
        KernelBase *kmh;

        shared_ptr<Future<int> > connection;

        unsigned writecount;
        unsigned readcount;

        bool pendingDequeue;
        bool pendingBlock;
        bool sentEnd;

        bool inread;
        bool incheckstatus;
    };
}

#endif
