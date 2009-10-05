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
 * \brief Implementation for the QueueWriter
 * \author John Bridgman
 */

#include "QueueWriter.h"
#include "NodeBase.h"
#include "Exceptions.h"

namespace CPN {

    QueueWriter::QueueWriter(NodeMessageHandler *n, WriterMessageHandler *wmh,
            Key_t writerkey, Key_t readerkey, shared_ptr<QueueBase> q)
        : WriterMessageHandler(wmh), nodeMsgHan(n), wkey(writerkey),
         rkey(readerkey), queue(q), shutdown(false)
    {
        readerMsgHan = queue->GetReaderMessageHandler();
        queue->SetWriterMessageHandler(this);
    }

    QueueWriter::~QueueWriter() {
    }

    void* QueueWriter::GetRawEnqueuePtr(unsigned thresh, unsigned chan) {
        Sync::AutoReentrantLock arl(queue->GetLock());
        if (shutdown) { throw BrokenQueueException(rkey); }
        nodeMsgHan->CheckTerminate();
        void* ptr = queue->GetRawEnqueuePtr(thresh, chan);
        while (0 == ptr) {
            readerMsgHan->RMHWriteBlock(wkey, rkey);
            arl.Unlock();
            nodeMsgHan->WriteBlock(wkey, rkey);
            arl.Lock();
            if (shutdown) { throw BrokenQueueException(rkey); }
            ptr = queue->GetRawEnqueuePtr(thresh, chan);
        }
        return ptr;
    }

    void QueueWriter::Enqueue(unsigned count) {
        Sync::AutoReentrantLock arl(queue->GetLock());
        queue->Enqueue(count);
        readerMsgHan->RMHEnqueue(wkey, rkey);
    }

    void QueueWriter::RawEnqueue(void *data, unsigned count,
            unsigned numChans, unsigned chanStride) {
        Sync::AutoReentrantLock arl(queue->GetLock());
        if (shutdown) { throw BrokenQueueException(rkey); }
        nodeMsgHan->CheckTerminate();
        while (!queue->RawEnqueue(data, count, numChans, chanStride)) {
            readerMsgHan->RMHWriteBlock(wkey, rkey);
            arl.Unlock();
            nodeMsgHan->WriteBlock(wkey, rkey);
            arl.Lock();
            if (shutdown) { throw BrokenQueueException(rkey); }
        }
        readerMsgHan->RMHEnqueue(wkey, rkey);
    }

    void QueueWriter::RawEnqueue(void *data, unsigned count) {
        Sync::AutoReentrantLock arl(queue->GetLock());
        if (shutdown) { throw BrokenQueueException(rkey); }
        nodeMsgHan->CheckTerminate();
        while (!queue->RawEnqueue(data, count)) {
            readerMsgHan->RMHWriteBlock(wkey, rkey);
            arl.Unlock();
            nodeMsgHan->WriteBlock(wkey, rkey);
            arl.Lock();
            if (shutdown) { throw BrokenQueueException(rkey); }
        }
        readerMsgHan->RMHEnqueue(wkey, rkey);
    }

    void QueueWriter::Shutdown() {
        Sync::AutoReentrantLock arl(queue->GetLock());
        if (!shutdown) {
            readerMsgHan->RMHEndOfWriteQueue(wkey, rkey);
            shutdown = true;
        }
    }

    void QueueWriter::Release() {
        Shutdown();
        nodeMsgHan->ReleaseWriter(wkey);
    }

    void QueueWriter::WMHEndOfReadQueue(Key_t src, Key_t dst) {
        Sync::AutoReentrantLock arl(queue->GetLock());
        shutdown = true;
        WriterMessageHandler::WMHEndOfReadQueue(src, dst);
    }
}

