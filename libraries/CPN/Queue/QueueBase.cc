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
 * \brief Implementation for the QueueBase.
 * \author John Bridgman
 */
#include "QueueBase.h"
#include "Exceptions.h"
#include "QueueAttr.h"
#include "Database.h"

namespace CPN {
    QueueBase::QueueBase(shared_ptr<Database> db, const SimpleQueueAttr &attr)
        : readerkey(attr.GetReaderKey()),
        writerkey(attr.GetWriterKey()),
        readshutdown(false),
        writeshutdown(false),
        readrequest(0),
        writerequest(0),
        database(db),
        datatype(attr.GetDatatype())
    {
    }

    QueueBase::~QueueBase() {}

    const void *QueueBase::GetRawDequeuePtr(unsigned thresh, unsigned chan) {
        database->CheckTerminated();
        Sync::AutoLock<QueueBase> al(*this);
        while (true) {
            const void *ptr = InternalGetRawDequeuePtr(thresh, chan);
            if (ptr || writeshutdown) { return ptr; }
            if (readshutdown) { throw BrokenQueueException(readerkey); }
            WaitForData(thresh);
        }
    }

    void QueueBase::Dequeue(unsigned count) {
        Sync::AutoLock<QueueBase> al(*this);
        if (readshutdown) { throw BrokenQueueException(readerkey); }
        InternalDequeue(count);
        NotifyFreespace();
    }

    bool QueueBase::RawDequeue(void* data, unsigned count, unsigned numChans, unsigned chanStride) {
        const void *src = GetRawDequeuePtr(count, 0);
        char *dest = (char*)data;
        if (!src) { return false; }
        memcpy(dest, src, count);
        for (unsigned chan = 1; chan < numChans; ++chan) {
            src = GetRawDequeuePtr(count, chan);
            ASSERT(src);
            dest += chanStride;
            memcpy(dest, src, count);
        }
        Dequeue(count);
        return true;
    }

    bool QueueBase::RawDequeue(void *data, unsigned count) {
        return RawDequeue(data, count, 1, 0);
    }

    void *QueueBase::GetRawEnqueuePtr(unsigned thresh, unsigned chan) {
        database->CheckTerminated();
        Sync::AutoLock<QueueBase> al(*this);
        while (true) {
            void *ptr = InternalGetRawEnqueuePtr(thresh, chan);
            if (ptr) { return ptr; }
            if (readshutdown || writeshutdown) { throw BrokenQueueException(writerkey); }
            WaitForFreespace(thresh);
        }
    }

    void QueueBase::Enqueue(unsigned count) {
        Sync::AutoLock<QueueBase> al(*this);
        if (writeshutdown) { throw BrokenQueueException(writerkey); }
        InternalEnqueue(count);
        NotifyData();
    }

    void QueueBase::RawEnqueue(const void *data, unsigned count, unsigned numChans, unsigned chanStride) {
        void *dest = GetRawEnqueuePtr(count, 0);
        const char *src = (char*)data;
        memcpy(dest, src, count);
        for (unsigned chan = 1; chan < numChans; ++chan) {
            dest = GetRawEnqueuePtr(count, chan);
            src += chanStride;
            memcpy(dest, src, count);
        }
        Enqueue(count);
    }


    void QueueBase::RawEnqueue(const void* data, unsigned count) {
        return RawEnqueue(data, count, 1, 0);
    }

    void QueueBase::ShutdownReader() {
        Sync::AutoLock<QueueBase> al(*this);
        readshutdown = true;
        cond.Broadcast();
    }

    void QueueBase::ShutdownWriter() {
        Sync::AutoLock<QueueBase> al(*this);
        writeshutdown = true;
        cond.Broadcast();
    }

    void QueueBase::WaitForData(unsigned requested) {
        readrequest = requested;
        while (Count() < readrequest && !(readshutdown || writeshutdown)) {
            database->CheckTerminated();
            cond.Wait(lock);
        }
        readrequest = 0;
    }

    void QueueBase::NotifyData() {
        if (Count() >= readrequest) {
            cond.Broadcast();
        }
    }

    void QueueBase::WaitForFreespace(unsigned requested) {
        writerequest = requested;
        while (Freespace() < writerequest && !(readshutdown || writeshutdown)) {
            database->CheckTerminated();
            cond.Wait(lock);
        }
        writerequest = 0;
    }

    void QueueBase::NotifyFreespace() {
        if (Freespace() >= writerequest) {
            cond.Broadcast();
        }
    }

    void QueueBase::NotifyTerminate() {
        cond.Broadcast();
    }

    QueueReleaser::~QueueReleaser() {}
}

