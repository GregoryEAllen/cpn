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

namespace CPN {
    
    class SocketEndpoint : public QueueBase, public SockHandler {
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

        Logger logger;

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
