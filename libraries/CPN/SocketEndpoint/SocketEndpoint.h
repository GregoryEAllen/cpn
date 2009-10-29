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
#include "QueueBase.h"
#include "SockHandler.h"
#include "PacketHeader.h"
#include "SimpleQueue.h"

namespace CPN {
    
    class SocketEndpoint 
        : public QueueBase, public SockHandler, private PacketHandler
    {
    public:

        enum Status_t {
            INIT,
            CONNECTING,
            LIVE,
            DIEING,
            DEAD
        };

        enum Mode_t {
            READ,
            WRITE
        }

        SocketEndpoint(Key_t readerkey, Key_t writerkey, Mode_t mode,
                KernelMessageHandler *kmh_);

        Status_t GetStatus() const;

        Mode_t GetMode() const { return mode; }
        Key_t GetWriterKey() const { return writerkey; }
        Key_t GetReaderKey() const { return readerkey; }
        Key_t GetKey() const { return mode == READ ? readerkey : writerkey; }

        /** Signal that the kernel is shutting down.
         */
        void Shutdown();

        /** Do any periodic things (like timing out a connection)
         * and/or return the maximum time we want to be checked again.
         */
        double CheckStatus();

        // From queuebase
        virtual const void* GetRawDequeuePtr(unsigned thresh, unsigned chan=0);
        virtual void Dequeue(unsigned count);
        virtual bool RawDequeue(void* data, unsigned count,
                unsigned numChans, unsigned chanStride);
        virtual bool RawDequeue(void* data, unsigned count);
		virtual void* GetRawEnqueuePtr(unsigned thresh, unsigned chan=0);
		virtual void Enqueue(unsigned count);
		virtual bool RawEnqueue(const void* data, unsigned count,
                unsigned numChans, unsigned chanStride);
		virtual bool RawEnqueue(const void* data, unsigned count);
        virtual unsigned NumChannels() const;
        virtual unsigned Count() const;
        virtual bool Empty() const;
		virtual unsigned Freespace() const;
		virtual bool Full() const;
        virtual unsigned MaxThreshold() const;
        virtual unsigned QueueLength() const;
        virtual void Grow(unsigned queueLen, unsigned maxThresh);

    protected:

        virtual void RMHEnqueue(Key_t writerkey, Key_t readerkey);
        virtual void RMHEndOfWriteQueue(Key_t writerkey, Key_t readerkey);
        virtual void RMHWriteBlock(Key_t writerkey, Key_t readerkey, unsigned requested);
        virtual void RMHTagChange(Key_t writerkey, Key_t readerkey);

        virtual void WMHDequeue(Key_t readerkey, Key_t writerkey);
        virtual void WMHEndOfReadQueue(Key_t readerkey, Key_t writerkey);
        virtual void WMHReadBlock(Key_t readerkey, Key_t writerkey, unsigned requested);
        virtual void WMHTagChange(Key_t readerkey, Key_t writerkey);

        virtual void OnRead();
        virtual void OnWrite();
        virtual void OnError();
        virtual void OnHup();
        virtual void OnInval();

    private:

        virtual void EnqueuePacket(const Packet &packet);
        virtual void DequeuePacket(const Packet &packet);
        virtual void ReadBlockPacket(const Packet &packet);
        virtual void WriteBlockPacket(const Packet &packet);
        virtual void EndOfWritePacket(const Packet &packet);
        virtual void EndOfReadPacket(const Packet &packet);
        virtual void IDReaderPacket(const Packet &packet);
        virtual void IDWriterPacket(const Packet &packet);

        Logger logger;
        ::SimpleQueue queue;

        Status_t status;
        const Mode_t mode;
        const Key_t writerkey;
        const Key_t readerkey;
        KernelMessageHandler *kmh;

        unsigned writecount;
        unsigned readcount;
    };
}

#endif
