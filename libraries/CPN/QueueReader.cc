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
#include "MessageQueue.h"
#include "Exceptions.h"

namespace CPN {

    QueueReader::QueueReader(QueueBlocker *n, Key_t k)
        : blocker(n), key(k), shutdown(false)
    {
    }

    QueueReader::~QueueReader() {
    }

    const void* QueueReader::GetRawDequeuePtr(unsigned thresh, unsigned chan) {
        CheckQueue();
        const void *ptr = queue->GetRawDequeuePtr(thresh, chan);
        while (0 == ptr) {
            if (shutdown) { return 0; }
            blocker->ReadBlock(shared_from_this(), thresh);
            ptr = queue->GetRawDequeuePtr(thresh, chan);
        }
        return ptr;
    }

    void QueueReader::Dequeue(unsigned count) {
        queue->Dequeue(count);
        PutMsg(NodeDequeue::Create(count));
    }

    bool QueueReader::RawDequeue(void *data, unsigned count,
            unsigned numChans, unsigned chanStride) {
        CheckQueue();
        while (!queue->RawDequeue(data, count, numChans, chanStride)) {
            if (shutdown) { return false; }
            blocker->ReadBlock(shared_from_this(), count);
        }
        PutMsg(NodeDequeue::Create(count));
        return true;
    }

    bool QueueReader::RawDequeue(void *data, unsigned count) {
        CheckQueue();
        while (!queue->RawDequeue(data, count)) {
            if (shutdown) { return false; }
            blocker->ReadBlock(shared_from_this(), count);
        }
        PutMsg(NodeDequeue::Create(count));
        return true;
    }

    void QueueReader::SetQueue(shared_ptr<QueueBase> q) {
        if (q) {
            if (shutdown) { throw BrokenQueueException(key); }
            queue = q;
            upstream = q->UpStreamChain();
            downstream = MsgMutator<NodeMessagePtr, KeyMutator>::Create(KeyMutator(key));
            downstream->Chain(blocker->GetMsgPut());
            q->DownStreamChain()->Chain(downstream);
        } else if (queue) {
            queue.reset();
            downstream.reset();
            upstream.reset();
        }
    }

    void QueueReader::Shutdown() {
        if (!shutdown) {
            if (queue) {
                PutMsg(NodeEndOfReadQueue::Create());
            }
            shutdown = true;
        }
    }

    void QueueReader::Release() {
        Shutdown();
        upstream.reset();
        downstream.reset();
        queue.reset();
        blocker->ReleaseReader(key);
    }
}

