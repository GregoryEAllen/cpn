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
#include <sstream>

namespace CPN {

    typedef AutoLock<QueueBase> AutoLock;

    QueueBase::QueueBase(shared_ptr<Database> db, const SimpleQueueAttr &attr)
        : readerkey(attr.GetReaderKey()),
        writerkey(attr.GetWriterKey()),
        readshutdown(false),
        writeshutdown(false),
        readrequest(0),
        writerequest(0),
        enqueuethresh(0),
        dequeuethresh(0),
        indequeue(false),
        inenqueue(false),
        database(db),
        useD4R(db->UseD4R()),
        logger(db.get(), Logger::DEBUG),
        datatype(attr.GetDatatype())
    {
        std::ostringstream oss;
        oss << "Queue(" << writerkey << ", " << readerkey << ")";
        logger.Name(oss.str());
    }

    QueueBase::~QueueBase() {}

    const void *QueueBase::GetRawDequeuePtr(unsigned thresh, unsigned chan) {
        database->CheckTerminated();
        AutoLock al(*this);
        if (indequeue) { ASSERT(dequeuethresh >= thresh); }
        else { dequeuethresh = thresh; }
        while (true) {
            const void *ptr = InternalGetRawDequeuePtr(thresh, chan);
            if (ptr || writeshutdown) {
                if (ptr) { indequeue = true; }
                return ptr;
            }
            if (readshutdown) { throw BrokenQueueException(readerkey); }
            if (thresh > MaxThreshold() && database->GrowQueueMaxThreshold()) {
                //printf("Grow(%u, %u)\n", 2*thresh, thresh);
                Grow(2*thresh, thresh);
            } else {
                readrequest = thresh;
                WaitForData();
                readrequest = 0;
            }
        }
    }

    void QueueBase::Dequeue(unsigned count) {
        AutoLock al(*this);
        dequeuethresh = 0;
        if (readshutdown) { throw BrokenQueueException(readerkey); }
        InternalDequeue(count);
        indequeue = false;
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
        AutoLock al(*this);
        if (inenqueue) { ASSERT(enqueuethresh >= thresh); }
        else { enqueuethresh = thresh; }
        while (true) {
            void *ptr = InternalGetRawEnqueuePtr(thresh, chan);
            if (ptr) {
                inenqueue = true;
                return ptr;
            }
            if (readshutdown || writeshutdown) { throw BrokenQueueException(writerkey); }
            if (thresh > MaxThreshold() && database->GrowQueueMaxThreshold()) {
                //printf("Grow(%u, %u)\n", 2*thresh, thresh);
                Grow(2*thresh, thresh);
            } else {
                writerequest = thresh;
                WaitForFreespace();
                writerequest = 0;
            }
        }
    }

    void QueueBase::Enqueue(unsigned count) {
        AutoLock al(*this);
        enqueuethresh = 0;
        if (writeshutdown) { throw BrokenQueueException(writerkey); }
        InternalEnqueue(count);
        inenqueue = false;
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
        AutoLock al(*this);
        readshutdown = true;
        cond.Broadcast();
    }

    void QueueBase::ShutdownWriter() {
        AutoLock al(*this);
        writeshutdown = true;
        cond.Broadcast();
    }

    void QueueBase::WaitForData() {
        if (useD4R) {
            ReadBlock();
        } else {
            while (ReadBlocked()) {
                cond.Wait(lock);
            }
        }
    }

    bool QueueBase::ReadBlocked() {
        database->CheckTerminated();
        return Count() < readrequest && !(readshutdown || writeshutdown);
    }

    void QueueBase::NotifyData() {
        if (Count() >= readrequest) {
            cond.Broadcast();
        }
    }

    void QueueBase::WaitForFreespace() {
        if (useD4R) {
            WriteBlock(QueueLength());
        } else {
            while (WriteBlocked()) {
                cond.Wait(lock);
            }
        }
    }

    bool QueueBase::WriteBlocked() {
        database->CheckTerminated();
        return Freespace() < writerequest && !(readshutdown || writeshutdown);
    }

    void QueueBase::NotifyFreespace() {
        if (Freespace() >= writerequest) {
            cond.Broadcast();
        }
    }

    void QueueBase::NotifyTerminate() {
        // Can't have the lock because this is called with the database lock
        cond.Broadcast();
    }

    void QueueBase::Detect() {
        AutoLock al(*this);
        unsigned size = database->CalculateGrowSize(Count(), writerequest);
        logger.Debug("Detect: Grow(%u, %u)", size, writerequest);
        Grow(size, writerequest);
        logger.Debug("New size: (%u, %u)", QueueLength(), MaxThreshold());
    }

    unsigned QueueBase::ReadRequest() {
        AutoLock al(*this);
        return readrequest;
    }

    unsigned QueueBase::WriteRequest() {
        AutoLock al(*this);
        return writerequest;
    }

    bool QueueBase::IsReaderShutdown() {
        AutoLock al(*this);
        return readshutdown;
    }

    bool QueueBase::IsWriterShutdown() {
        AutoLock al(*this);
        return writeshutdown;
    }

    void QueueBase::LogState() {
        logger.Error("Printing state (w:%llu r:%llu)", readerkey, writerkey);
        logger.Error("size: %u, maxthresh: %u count: %u free: %u",
                QueueLength(), MaxThreshold(), Count(), Freespace());
        logger.Error("readrequest: %u, writerequest: %u", readrequest, writerequest);
        if (indequeue) {
            logger.Error("Indequeue (thresh: %u)", dequeuethresh);
        }
        if (inenqueue) {
            logger.Error("Inenqueue (thresh: %u)", enqueuethresh);
        }
        if (readshutdown) {
            logger.Error("Reader shutdown");
        }
        if (writeshutdown) {
            logger.Error("Writer shutdown");
        }
    }

    QueueReleaser::~QueueReleaser() {}
}

