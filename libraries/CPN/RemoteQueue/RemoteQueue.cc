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
#include "RemoteQueue.h"
#include "RemoteQueueHolder.h"
#include "QueueAttr.h"
#include "Database.h"
#include "ErrnoException.h"
#include "AutoLock.h"
#include <errno.h>
#include <algorithm>
#include <sstream>

#if 0
#define FUNC_TRACE(logger) logger.Trace("%s (c: %llu)", __PRETTY_FUNCTION__, clock.Get())
#else
#define FUNC_TRACE(logger)
#endif


namespace CPN {

    RemoteQueue::RemoteQueue(shared_ptr<Database> db, Mode_t mode_,
                ConnectionServer *s, RemoteQueueHolder *h, const SimpleQueueAttr &attr)
        : ThresholdQueue(db, attr, QueueLength(attr.GetLength(), attr.GetMaxThreshold(), attr.GetAlpha(), mode_)),
        mode(mode_),
        alpha(attr.GetAlpha()),
        server(s),
        holder(h),
        mocknode(mode_ == READ ? attr.GetReaderNodeKey() : attr.GetWriterNodeKey()),
        readerlength(QueueLength(attr.GetLength(), attr.GetMaxThreshold(), attr.GetAlpha(), READ)),
        writerlength(QueueLength(attr.GetLength(), attr.GetMaxThreshold(), attr.GetAlpha(), WRITE)),
        bytecount(0),
        pendingBlock(false),
        sentEnd(false),
        pendingGrow(false),
        pendingD4RTag(false),
        dead(false)
    {
        if (mode == READ) {
            SetWriterNode(&mocknode);
        } else {
            SetReaderNode(&mocknode);
        }
        std::ostringstream oss;
        oss << "RemoteQueue(m:";
        if (mode == READ) { oss << "r"; }
        else { oss << "w"; }
        oss << ", r:" << readerkey << ", w:" << writerkey << ")";
        logger.Name(oss.str());
        logger.Trace("Constructed (c: %llu)", clock.Get());
    }

    RemoteQueue::~RemoteQueue() {
        logger.Trace("Destructed (c: %llu)", clock.Get());
    }

    void RemoteQueue::Shutdown() {
        Sync::AutoLock<const QueueBase> al(*this);
        dead = true;
        sock.Close();
    }

    unsigned RemoteQueue::Count() const {
        Sync::AutoLock<const QueueBase> al(*this);
        if (mode == READ) {
            return queue->Count();
        } else {
            return queue->Count() + bytecount;
        }
    }

    bool RemoteQueue::Empty() const {
        Sync::AutoLock<const QueueBase> al(*this);
        if (mode == READ) {
            return queue->Empty();
        } else {
            return queue->Empty() && (bytecount == 0);
        }
    }

    unsigned RemoteQueue::QueueLength() const {
        Sync::AutoLock<const QueueBase> al(*this);
        return readerlength + writerlength;
    }

    void RemoteQueue::Grow(unsigned queueLen, unsigned maxThresh) {
        Sync::AutoLock<QueueBase> al(*this);
        const unsigned maxthresh = std::max<unsigned>(queue->MaxThreshold(), maxThresh);
        readerlength = QueueLength(queueLen, maxthresh, alpha, READ);
        writerlength = QueueLength(queueLen, maxthresh, alpha, WRITE);
        const unsigned newlen = (mode == WRITE ? writerlength : readerlength);
        ThresholdQueue::Grow(newlen, maxthresh);
        pendingGrow = true;
        InternalCheckStatus();
    }

    void RemoteQueue::ShutdownReader() {
        ASSERT(mode == READ);
        ThresholdQueue::ShutdownReader();
        InternalCheckStatus();
    }

    void RemoteQueue::ShutdownWriter() {
        ASSERT(mode == WRITE);
        ThresholdQueue::ShutdownWriter();
        InternalCheckStatus();
    }

    void RemoteQueue::WaitForData() {
        ASSERT(mode == READ);
        pendingBlock = true;
        InternalCheckStatus();
        sock.Writeable(false);
        ThresholdQueue::WaitForData();
    }

    void RemoteQueue::WaitForFreespace() {
        ASSERT(mode == WRITE);
        pendingBlock = true;
        InternalCheckStatus();
        ThresholdQueue::WaitForFreespace();
    }

    void RemoteQueue::InternalDequeue(unsigned count) {
        ASSERT(mode == READ);
        ThresholdQueue::InternalDequeue(count);
        InternalCheckStatus();
    }

    void RemoteQueue::InternalEnqueue(unsigned count) {
        ASSERT(mode == WRITE);
        ThresholdQueue::InternalEnqueue(count);
        InternalCheckStatus();
    }

    void RemoteQueue::SignalReaderTagChanged() {
        Sync::AutoLock<QueueBase> al(*this);
        ASSERT(mode == READ);
        pendingD4RTag = true;
        ThresholdQueue::SignalReaderTagChanged();
        InternalCheckStatus();
    }

    void RemoteQueue::SignalWriterTagChanged() {
        Sync::AutoLock<QueueBase> al(*this);
        ASSERT(mode == WRITE);
        pendingD4RTag = true;
        ThresholdQueue::SignalWriterTagChanged();
        InternalCheckStatus();
    }

    void RemoteQueue::SetupPacket(Packet &packet) {
        packet.Clock(clock.Get());
        packet.NumChannels(queue->NumChannels());
    }

    void RemoteQueue::EnqueuePacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE(logger);
        ASSERT(mode == READ);
        ASSERT(!writeshutdown);
        unsigned count = packet.Count();
        if (count > queue->Freespace() || count > queue->MaxThreshold()) {
            logger.Warn("Enqueue packet too large, silently growing queue");
            ThresholdQueue::Grow(queue->Count() + count, count);
        }
        std::vector<iovec> iovs;
        for (unsigned i = 0; i < packet.NumChannels(); ++i) {
            iovec iov;
            iov.iov_base = queue->GetRawEnqueuePtr(count, i);
            ASSERT(iov.iov_base, "Internal throttle failed! (c: %llu)", clock.Get());
            iov.iov_len = count;
            iovs.push_back(iov);
        }
        unsigned numread = sock.Readv(&iovs[0], iovs.size());
        ASSERT(numread == packet.DataLength());
        queue->Enqueue(count);
        bytecount += count;
        writerequest = 0;
        NotifyData();
    }

    void RemoteQueue::SendEnqueuePacket() {
        FUNC_TRACE(logger);
        unsigned count = queue->Count();
        const unsigned maxthresh = queue->MaxThreshold();
        const unsigned expectedfree = std::max(readerlength, maxthresh) - bytecount;
        if (count > maxthresh) { count = maxthresh; }
        if (count > expectedfree) { count = expectedfree; }
        if (count == 0) { return; }
        const unsigned datalength = count * queue->NumChannels();
        Packet packet(datalength, PACKET_ENQUEUE);
        SetupPacket(packet);
        packet.Count(count);
        PacketEncoder::SendEnqueue(packet, *queue);
        bytecount += count;
        QueueBase::NotifyFreespace();
    }

    void RemoteQueue::DequeuePacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE(logger);
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        readrequest = 0;
        bytecount -= packet.Count();
    }

    void RemoteQueue::SendDequeuePacket() {
        FUNC_TRACE(logger);
        Packet packet(PACKET_DEQUEUE);
        SetupPacket(packet);
        const unsigned count = bytecount - queue->Count();
        packet.Count(count);
        PacketEncoder::SendPacket(packet);
        bytecount -= count;
    }

    void RemoteQueue::ReadBlockPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE(logger);
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        readrequest = packet.Requested();
        if (useD4R) {
            pendingD4RTag = true;
        }
    }

    void RemoteQueue::SendReadBlockPacket() {
        FUNC_TRACE(logger);
        Packet packet(PACKET_READBLOCK);
        SetupPacket(packet);
        packet.Requested(readrequest);
        PacketEncoder::SendPacket(packet);
    }

    void RemoteQueue::WriteBlockPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE(logger);
        ASSERT(mode == READ);
        ASSERT(!writeshutdown);
        writerequest = packet.Requested();
        if (useD4R) {
            pendingD4RTag = true;
        }
    }

    void RemoteQueue::SendWriteBlockPacket() {
        FUNC_TRACE(logger);
        Packet packet(PACKET_WRITEBLOCK);
        SetupPacket(packet);
        packet.Requested(writerequest);
        PacketEncoder::SendPacket(packet);
    }

    void RemoteQueue::EndOfWritePacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE(logger);
        ASSERT(mode == READ);
        ASSERT(!writeshutdown);
        QueueBase::ShutdownWriter();
    }

    void RemoteQueue::SendEndOfWritePacket() {
        FUNC_TRACE(logger);
        Packet packet(PACKET_ENDOFWRITE);
        SetupPacket(packet);
        PacketEncoder::SendPacket(packet);
        sock.ShutdownWrite();
    }

    void RemoteQueue::EndOfReadPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE(logger);
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        QueueBase::ShutdownReader();
    }

    void RemoteQueue::SendEndOfReadPacket() {
        FUNC_TRACE(logger);
        Packet packet(PACKET_ENDOFREAD);
        SetupPacket(packet);
        PacketEncoder::SendPacket(packet);
        sock.ShutdownWrite();
    }

    void RemoteQueue::GrowPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE(logger);
        const unsigned queueLen = packet.QueueSize();
        const unsigned maxthresh = std::max<unsigned>(queue->MaxThreshold(), packet.MaxThreshold());
        readerlength = QueueLength(queueLen, maxthresh, alpha, READ);
        writerlength = QueueLength(queueLen, maxthresh, alpha, WRITE);
        const unsigned newlen = (mode == WRITE ? writerlength : readerlength);
        ThresholdQueue::Grow(newlen, maxthresh);
    }

    void RemoteQueue::SendGrowPacket() {
        FUNC_TRACE(logger);
        Packet packet(PACKET_GROW);
        SetupPacket(packet);
        packet.QueueSize(readerlength + writerlength);
        packet.MaxThreshold(queue->MaxThreshold());
        PacketEncoder::SendPacket(packet);
    }

    void RemoteQueue::D4RTagPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE(logger);
        ASSERT(packet.DataLength() == sizeof(D4R::Tag));
        D4R::Tag tag;
        unsigned numread = sock.Read(&tag, sizeof(tag));
        ASSERT(numread == sizeof(tag));
        mocknode.SetPublicTag(tag);
        if (mode == WRITE) {
            QueueBase::SignalReaderTagChanged();
        } else {
            QueueBase::SignalWriterTagChanged();
        }
    }

    void RemoteQueue::SendD4RTagPacket() {
        FUNC_TRACE(logger);
        Packet packet(PACKET_D4RTAG);
        SetupPacket(packet);
        packet.DataLength(sizeof(D4R::Tag));
        D4R::Tag tag;
        if (mode == READ) {
            tag = reader->GetPublicTag();
        } else {
            tag = writer->GetPublicTag();
        }
        PacketEncoder::SendPacket(packet, &tag);
    }

    void RemoteQueue::IDReaderPacket(const Packet &packet) {
        ASSERT(false, "Unexpected packet");
    }

    void RemoteQueue::IDWriterPacket(const Packet &packet) {
        ASSERT(false, "Unexpected packet");
    }

    void RemoteQueue::Read() {
        while (sock.Readable()) {
            unsigned numtoread = 0;
            void *ptr = PacketDecoder::GetDecoderBytes(numtoread);
            unsigned numread = sock.Recv(ptr, numtoread, false);
            if (numread == 0) {
                if (sock.Eof()) {
                    logger.Debug("Read EOF");
                }
                break;
            } else {
                PacketDecoder::ReleaseDecoderBytes(numread);
            }
        }
    }

    void RemoteQueue::WriteBytes(const iovec *iov, unsigned iovcnt) {
        unsigned numwritten = sock.Writev(iov, iovcnt);
#if 1
        unsigned total = 0;
        for (unsigned i = 0; i < iovcnt; ++i) { total += iov[i].iov_len; }
        ASSERT(total == numwritten);
#endif
    }

    void *RemoteQueue::EntryPoint() {
        try {
            while (true) {
                try {
                    if (database->IsTerminated()) {
                        logger.Debug("Forced Shutdown (c: %llu)", clock.Get());
                        break;
                    }
                    {
                        Sync::AutoLock<QueueBase> al(*this);
                        if (dead) {
                            logger.Debug("Shutdown (c: %llu)", clock.Get());
                            break;
                        }
                    }
                    if (sock.Closed()) {
                        logger.Debug("Connecting (c: %llu)", clock.Get());
                        shared_ptr<Future<int> > conn;
                        if (mode == WRITE) {
                            conn = server->ConnectWriter(GetKey());
                        } else {
                            conn = server->ConnectReader(GetKey());
                        }
                        if (conn) {
                            sock.Reset();
                            sock.FD(conn->Get());
                        }
                        if (sock.Closed()) {
                            logger.Debug("Connection Failed (c: %llu)", clock.Get());
                        } else {
                            logger.Debug("Connected (c: %llu)", clock.Get());
                        }
                    } else {
                        FileHandle *fds[2];
                        fds[0] = &sock;
                        fds[1] = holder->GetWakeup();
                        FileHandle::Poll(fds, fds+2, -1);
                        InternalCheckStatus();
                    }
                } catch (const ErrnoException &e) {
                    logger.Error("Exception in RemoteQueue main loop (c: %llu e: %d): %s",
                            clock.Get(), e.Error(), e.what());
                    HandleError(e.Error());
                }
            }
        } catch (...) {
            holder->CleanupQueue(GetKey());
            throw;
        }
        holder->CleanupQueue(GetKey());
        server->Wakeup();
        return 0;
    }

    void RemoteQueue::InternalCheckStatus() {
        Sync::AutoLock<QueueBase> al(*this);
        if (sock.Closed()) {
            return;
        }
        if (database->IsTerminated()) {
            if (mode == WRITE) {
                ThresholdQueue::ShutdownWriter();
            } else {
                ThresholdQueue::ShutdownReader();
            }
        }

        clock.Tick();

        if (sock.Eof() && !(readshutdown || writeshutdown)) {
            logger.Error("Eof detected but not shutdown! (c: %llu)", clock.Get());
            ASSERT(false, "EOF detected but not shutdown! (c: %llu)", clock.Get());
        }

        try {
            Read();

            if (pendingGrow) {
                SendGrowPacket();
                pendingGrow = false;
            }

            if (mode == WRITE) {
                if (!queue->Empty()) {
                    // If we can write more try to write more
                    if (bytecount < readerlength) {
                        SendEnqueuePacket();
                    }
                    // If we can still write more, have the next call to Poll
                    // check writeability
                    if (bytecount < readerlength) {
                        sock.Writeable(false);
                    }
                }
                // A pending block is present
                if (pendingBlock) {
                    // If we have received dequeue packets
                    // sense the block happened don't bother sending anything
                    if (writerequest > queue->Freespace()) {
                        SendWriteBlockPacket();
                    }
                    pendingBlock = false;
                }

                if (pendingD4RTag) {
                    if (writer && !writeshutdown) {
                        SendD4RTagPacket();
                        pendingD4RTag = false;
                    }
                }

                if (writeshutdown) {
                    if (!sentEnd && queue->Empty()) {
                        SendEndOfWritePacket();
                        sentEnd = true;
                    }
                }

                if (readshutdown) {
                    if (!sentEnd) {
                        SendEndOfWritePacket();
                        sentEnd = true;
                    }
                    logger.Debug("Closing the socket (c: %llu)", clock.Get());
                    sock.Close();
                    dead = true;
                }
            } else {
                // If some bytes have been read from the queue
                if (bytecount > queue->Count()) {
                    SendDequeuePacket();
                }

                if (pendingBlock) {
                    // May have received enqueue packets...
                    if (readrequest > queue->Count()) {
                        SendReadBlockPacket();
                    }
                    pendingBlock = false;
                }
                if (pendingD4RTag) {
                    if (reader && !readshutdown) {
                        SendD4RTagPacket();
                        pendingD4RTag = false;
                    }
                }
                if (readshutdown) {
                    if (!sentEnd) {
                        SendEndOfReadPacket();
                        sentEnd = true;
                    }
                }
                if (writeshutdown) {
                    if (!sentEnd) {
                        SendEndOfReadPacket();
                        sentEnd = true;
                    }
                    logger.Debug("Closing the socket (c: %llu)", clock.Get());
                    sock.Close();
                    dead = true;
                }
            }
        } catch (const ErrnoException &e) {
            logger.Error("Exception in RemoteQueue check status (c: %llu e: %d): %s",
                    clock.Get(), e.Error(), e.what());
            HandleError(e.Error());
        }
    }

    void RemoteQueue::HandleError(int error) {
        Sync::AutoLock<QueueBase> al(*this);
        if (readshutdown || writeshutdown) {
            readshutdown = writeshutdown = true;
            Signal();
        }
        switch (error) {
        case EPIPE:
        case EBADF:
            sock.Close();
            break;
        default:
            break;
        }
    }

    unsigned RemoteQueue::QueueLength(unsigned length, unsigned maxthresh, double alpha, Mode_t mode) {
        unsigned writerlen = unsigned(((double)length)*alpha);
        if (mode == READ) {
            return std::max<unsigned>(length - writerlen, maxthresh);
        } else {
            return std::max<unsigned>(writerlen, maxthresh);
        }
    }

    const char *BoolString(bool tf) {
        if (tf) {
            return "true";
        } else {
            return "false";
        }
    }

    void RemoteQueue::LogState() {
        ThresholdQueue::LogState();
        logger.Error("Mode: %s, Readerlength: %u, Writerlength %u, Clock: %llu, bytecount: %u",
                mode == READ ? "read" : "write", readerlength, writerlength, clock.Get(), bytecount);
        logger.Error("PendingBlock: %s, SentEnd: %s, PendingGrow: %s, PendingD4R: %s, Dead: %s, Running: %s",
                BoolString(pendingBlock), BoolString(sentEnd), BoolString(pendingGrow),
                BoolString(pendingD4RTag), BoolString(dead), BoolString(Running()));
        logger.Error("Thread id: %llu", (unsigned long long)((pthread_t)(*this)));
        if (sock.Closed()) {
            logger.Error("Socket closed");
        }
    }
}

