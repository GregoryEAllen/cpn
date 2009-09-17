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
#include "MessageQueue.h"
#include "Exceptions.h"

namespace CPN {

    QueueWriter::QueueWriter(QueueBlocker* n, Key_t k)
        : blocker(n), key(k), shutdown(false) {}

    QueueWriter::~QueueWriter() {
    }

    void* QueueWriter::GetRawEnqueuePtr(unsigned thresh, unsigned chan) {
        if (shutdown) { throw BrokenQueueException(key); }
        CheckQueue();
        void* ptr = queue->GetRawEnqueuePtr(thresh, chan);
        while (0 == ptr) {
            blocker->WriteBlock(shared_from_this(), thresh);
            if (shutdown) { throw BrokenQueueException(key); }
            ptr = queue->GetRawEnqueuePtr(thresh, chan);
        }
        return ptr;
    }

    void QueueWriter::Enqueue(unsigned count) {
        queue->Enqueue(count);
        PutMsg(NodeEnqueue::Create(count));
    }

    void QueueWriter::RawEnqueue(void *data, unsigned count,
            unsigned numChans, unsigned chanStride) {
        if (shutdown) { throw BrokenQueueException(key); }
        CheckQueue();
        while (!queue->RawEnqueue(data, count, numChans, chanStride)) {
            blocker->WriteBlock(shared_from_this(), count);
            if (shutdown) { throw BrokenQueueException(key); }
        }
        PutMsg(NodeEnqueue::Create(count));
    }

    void QueueWriter::RawEnqueue(void *data, unsigned count) {
        if (shutdown) { throw BrokenQueueException(key); }
        CheckQueue();
        while (!queue->RawEnqueue(data, count)) {
            blocker->WriteBlock(shared_from_this(), count);
            if (shutdown) { throw BrokenQueueException(key); }
        }
        PutMsg(NodeEnqueue::Create(count));
    }

    void QueueWriter::SetQueue(shared_ptr<QueueBase> q) {
        if (q) {
            if (shutdown) { throw BrokenQueueException(key); }
            queue = q;
            downstream = q->DownStreamChain();
            // chain messages from the reader to our upstream queue
            upstream = MsgMutator<NodeMessagePtr, KeyMutator>::Create(KeyMutator(key));
            upstream->Chain(blocker->GetMsgPut());
            q->UpStreamChain()->Chain(upstream);
        } else if (queue) {
            queue.reset();
            downstream.reset();
            upstream.reset();
        }
    }

    void QueueWriter::Shutdown() {
        if (!shutdown) {
            if (queue) {
                PutMsg(NodeEndOfWriteQueue::Create());
            }
            shutdown = true;
        }
    }

    void QueueWriter::Release() {
        Shutdown();
        downstream.reset();
        upstream.reset();
        queue.reset();
        blocker->ReleaseWriter(key);
    }
}

