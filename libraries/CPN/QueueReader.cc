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
 * \brief Implemenation for the QueueReader
 * \author John Bridgman
 */

#include "QueueReader.h"
#include "NodeBase.h"
#include "Exceptions.h"

namespace CPN {

    QueueReader::QueueReader(NodeMessageHandler *n,
            ReaderMessageHandler *rmh,
            Key_t readerkey,
            Key_t writerkey,
            shared_ptr<QueueBase> q)
        : ReaderMessageHandler(rmh), nodeMsgHan(n), rkey(readerkey), wkey(writerkey),
        queue(q), shutdown(false)
    {
        writerMsgHan = queue->GetWriterMessageHandler();
        queue->SetReaderMessageHandler(this);
    }

    QueueReader::~QueueReader() {
        Shutdown();
        queue->ClearReaderMessageHandler();
        SetSubReaderHandler(0);
    }

    const void* QueueReader::GetRawDequeuePtr(unsigned thresh, unsigned chan) {
        nodeMsgHan->CheckTerminate();
        Sync::AutoReentrantLock arl(queue->GetLock());
        const void *ptr = queue->GetRawDequeuePtr(thresh, chan);
        while (0 == ptr) {
            if (shutdown) { return 0; }
            writerMsgHan->WMHReadBlock(rkey, wkey, thresh);
            arl.Unlock();
            nodeMsgHan->ReadBlock(rkey, wkey);
            arl.Lock();
            ptr = queue->GetRawDequeuePtr(thresh, chan);
        }
        return ptr;
    }

    void QueueReader::Dequeue(unsigned count) {
        Sync::AutoReentrantLock arl(queue->GetLock());
        queue->Dequeue(count);
        writerMsgHan->WMHDequeue(rkey, wkey);
    }

    bool QueueReader::RawDequeue(void *data, unsigned count,
            unsigned numChans, unsigned chanStride) {
        nodeMsgHan->CheckTerminate();
        Sync::AutoReentrantLock arl(queue->GetLock());
        while (!queue->RawDequeue(data, count, numChans, chanStride)) {
            if (shutdown) { return false; }
            writerMsgHan->WMHReadBlock(rkey, wkey, count);
            arl.Unlock();
            nodeMsgHan->ReadBlock(rkey, wkey);
            arl.Lock();
        }
        writerMsgHan->WMHDequeue(rkey, wkey);
        return true;
    }

    bool QueueReader::RawDequeue(void *data, unsigned count) {
        nodeMsgHan->CheckTerminate();
        Sync::AutoReentrantLock arl(queue->GetLock());
        while (!queue->RawDequeue(data, count)) {
            if (shutdown) { return false; }
            writerMsgHan->WMHReadBlock(rkey, wkey, count);
            arl.Unlock();
            nodeMsgHan->ReadBlock(rkey, wkey);
            arl.Lock();
        }
        writerMsgHan->WMHDequeue(rkey, wkey);
        return true;
    }

    void QueueReader::Shutdown() {
        Sync::AutoReentrantLock arl(queue->GetLock());
        if (!shutdown) {
            writerMsgHan->WMHEndOfReadQueue(rkey, wkey);
            shutdown = true;
        }
    }

    void QueueReader::Release() {
        Shutdown();
        nodeMsgHan->ReleaseReader(rkey);
    }

    void QueueReader::RMHEndOfWriteQueue(Key_t src, Key_t dst) {
        Sync::AutoReentrantLock arl(queue->GetLock());
        shutdown = true;
        ReaderMessageHandler::RMHEndOfWriteQueue(src, dst);
    }
}

