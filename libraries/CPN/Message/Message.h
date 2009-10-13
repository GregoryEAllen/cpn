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

#ifndef CPN_MESSAGE_H
#define CPN_MESSAGE_H
#pragma once
#include "CPNCommon.h"
#include "Assert.h"
#include "AsyncStream.h"

namespace CPN {

    /**
     * Handler of messages to the reader
     */
    class ReaderMessageHandler {
    public:
        ReaderMessageHandler();
        ReaderMessageHandler(ReaderMessageHandler *shan);
        virtual ~ReaderMessageHandler();
        virtual void RMHEnqueue(Key_t writerkey, Key_t readerkey);
        virtual void RMHEndOfWriteQueue(Key_t writerkey, Key_t readerkey);
        virtual void RMHWriteBlock(Key_t writerkey, Key_t readerkey, unsigned requested);
        virtual void RMHTagChange(Key_t writerkey, Key_t readerkey);
    protected:
        ReaderMessageHandler *GetSubReaderHandler() const { return subhandler; }
        void SetSubReaderHandler(ReaderMessageHandler *shan) { subhandler = shan; }
        virtual bool CheckRMH() { return subhandler; }
    private:
        ReaderMessageHandler *subhandler;
    };

    class WriterMessageHandler {
    public:
        WriterMessageHandler();
        WriterMessageHandler(WriterMessageHandler *shan);
        virtual ~WriterMessageHandler();
        virtual void WMHDequeue(Key_t readerkey, Key_t writerkey);
        virtual void WMHEndOfReadQueue(Key_t readerkey, Key_t writerkey);
        virtual void WMHReadBlock(Key_t readerkey, Key_t writerkey, unsigned requested);
        virtual void WMHTagChange(Key_t readerkey, Key_t writerkey);
    protected:
        WriterMessageHandler *GetSubWriterHandler() const { return subhandler; }
        void SetSubWriterHandler(WriterMessageHandler *shan) { subhandler = shan; }
        virtual bool CheckWMH() { return subhandler; }
    private:
        WriterMessageHandler *subhandler;
    };

    class NodeMessageHandler {
    public:
        virtual ~NodeMessageHandler();
        virtual void Shutdown() = 0;
        virtual void CreateReader(Key_t readerkey, Key_t writerkey, shared_ptr<QueueBase> q) = 0;
        virtual void CreateWriter(Key_t readerkey, Key_t writerkey, shared_ptr<QueueBase> q) = 0;
        virtual void ReadBlock(Key_t readerkey, Key_t writerkey) = 0;
        virtual void WriteBlock(Key_t writerkey, Key_t readerkey) = 0;
        virtual void ReleaseWriter(Key_t key) = 0;
        virtual void ReleaseReader(Key_t key) = 0;
        virtual void CheckTerminate() = 0;
    };

    class KernelMessageHandler {
    public:
        virtual ~KernelMessageHandler();
        virtual void CreateWriter(Key_t dst, const SimpleQueueAttr &attr);
        virtual void CreateReader(Key_t dst, const SimpleQueueAttr &attr);
        virtual void CreateQueue(Key_t dst, const SimpleQueueAttr &attr);
        virtual void CreateNode(Key_t dst, const NodeAttr &attr);

        // Functions the streams need of the kernel
        virtual void StreamDead(Key_t streamkey);
        virtual void SetReaderDescriptor(Key_t readerkey, Key_t writerkey, Async::DescriptorPtr desc);
        virtual void SetWriterDescriptor(Key_t writerkey, Key_t readerkey, Async::DescriptorPtr desc);
        virtual weak_ptr<UnknownStream> CreateNewQueueStream(Key_t readerkey, Key_t writerkey);
        virtual void NewKernelStream(Key_t kernelkey, Async::DescriptorPtr desc);
        virtual void SendWakeup();
    };
}

#endif
