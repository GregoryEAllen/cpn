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
#include "Exceptions.h"
#include "AutoLock.h"
#include "PthreadFunctional.h"
#include "ConnectionServer.h"
#include "ErrnoException.h"
#include "Assert.h"
#include "D4RNode.h"
#include <errno.h>
#include <algorithm>
#include <sstream>

#if _DEBUG
#define FUNC_TRACE(logger) logger.Trace("%s %s", __PRETTY_FUNCTION__, GetState().c_str())
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
        mocknode(new D4R::Node(mode_ == READ ? attr.GetReaderNodeKey() : attr.GetWriterNodeKey())),
        readerlength(QueueLength(attr.GetLength(), attr.GetMaxThreshold(), attr.GetAlpha(), READ)),
        writerlength(QueueLength(attr.GetLength(), attr.GetMaxThreshold(), attr.GetAlpha(), WRITE)),
        bytecount(0),
        pendingBlock(false),
        sentEnd(false),
        pendingGrow(false),
        pendingD4RTag(false),
        tagUpdated(false),
        dead(false)
    {
        fileThread = auto_ptr<Pthread>(CreatePthreadFunctional(this, &RemoteQueue::FileThreadEntryPoint));
        actionThread = auto_ptr<Pthread>(CreatePthreadFunctional(this, &RemoteQueue::ActionThreadEntryPoint));
        if (mode == READ) {
            SetWriterNode(mocknode);
        } else {
            SetReaderNode(mocknode);
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
        ASSERT_ABORT(dead, "Shutdown but not dead!?");
        Signal();
        logger.Trace("Destructed (c: %llu)", clock.Get());
        fileThread.reset();
        actionThread.reset();
    }

    void RemoteQueue::Start() {
        fileThread->Start();
    }

    void RemoteQueue::Shutdown() {
        AutoLock<const QueueBase> al(*this);
        dead = true;
        Signal();
    }

    unsigned RemoteQueue::Count() const {
        AutoLock<const QueueBase> al(*this);
        if (mode == READ) {
            return queue->Count();
        } else {
            return queue->Count() + bytecount;
        }
    }

    bool RemoteQueue::Empty() const {
        AutoLock<const QueueBase> al(*this);
        if (mode == READ) {
            return queue->Empty();
        } else {
            return queue->Empty() && (bytecount == 0);
        }
    }

    unsigned RemoteQueue::QueueLength() const {
        AutoLock<const QueueBase> al(*this);
        return readerlength + writerlength;
    }

    void RemoteQueue::Grow(unsigned queueLen, unsigned maxThresh) {
        AutoLock<QueueBase> al(*this);
        const unsigned maxthresh = std::max<unsigned>(queue->MaxThreshold(), maxThresh);
        readerlength = QueueLength(queueLen, maxthresh, alpha, READ);
        writerlength = QueueLength(queueLen, maxthresh, alpha, WRITE);
        const unsigned newlen = (mode == WRITE ? writerlength : readerlength);
        ThresholdQueue::Grow(newlen, maxthresh);
        pendingGrow = true;
        Signal();
    }

    void RemoteQueue::WaitForData() {
        ASSERT(mode == READ);
        pendingBlock = true;
        tagUpdated = false;
        Signal();
        while (ReadBlocked() && !tagUpdated) {
            Wait();
        }
        if (ReadBlocked() && tagUpdated) {
            ThresholdQueue::WaitForData();
        }
    }

    void RemoteQueue::WaitForFreespace() {
        ASSERT(mode == WRITE);
        pendingBlock = true;
        tagUpdated = false;
        Signal();
        while (WriteBlocked() && !tagUpdated) {
            Wait();
        }
        if (WriteBlocked() && tagUpdated) {
            ThresholdQueue::WaitForFreespace();
        }
    }

    void RemoteQueue::InternalDequeue(unsigned count) {
        ASSERT(mode == READ);
        FUNC_TRACE(logger);
        ThresholdQueue::InternalDequeue(count);
        Signal();
    }

    void RemoteQueue::InternalEnqueue(unsigned count) {
        ASSERT(mode == WRITE);
        FUNC_TRACE(logger);
        ThresholdQueue::InternalEnqueue(count);
        Signal();
    }

    void RemoteQueue::SignalReaderTagChanged() {
        AutoLock<QueueBase> al(*this);
        ASSERT(mode == READ);
        pendingD4RTag = true;
        ThresholdQueue::SignalReaderTagChanged();
    }

    void RemoteQueue::SignalWriterTagChanged() {
        AutoLock<QueueBase> al(*this);
        ASSERT(mode == WRITE);
        pendingD4RTag = true;
        ThresholdQueue::SignalWriterTagChanged();
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
        unsigned numread = 0;
        unsigned numtoread = packet.DataLength();
        unsigned i = 0;
        while (numread < numtoread) {
            unsigned num = sock.Readv(&iovs[i], iovs.size() - i);
            numread += num;
            if (numread == numtoread) break;
            while (iovs[i].iov_len <= num) {
                num -= iovs[i].iov_len;
                ++i;
            }
            iovs[i].iov_len -= num;
            iovs[i].iov_base = ((char*)iovs[i].iov_base) + num;
        }
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
        if (readrequest > queue->Count() + bytecount) {
            if (useD4R) {
                pendingD4RTag = true;
            }
        }
        Signal();
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
        if (writerequest > readerlength - queue->Count() || queue->Count() > readerlength) {
            if (useD4R) {
                pendingD4RTag = true;
            }
        }
        Signal();
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
        ASSERT(!sentEnd);
        Packet packet(PACKET_ENDOFWRITE);
        SetupPacket(packet);
        PacketEncoder::SendPacket(packet);
        try {
            sock.ShutdownWrite();
        } catch (const ErrnoException &e) {
            logger.Debug("Error trying to close the write end: %s", e.what());
        }
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
        ASSERT(!sentEnd);
        Packet packet(PACKET_ENDOFREAD);
        SetupPacket(packet);
        PacketEncoder::SendPacket(packet);
        try {
            sock.ShutdownWrite();
        } catch (const ErrnoException &e) {
            logger.Debug("Error trying to close the read end: %s", e.what());
        }
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
        tagUpdated = true;
        Signal();
        mocknode->SetPublicTag(tag);
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
        unsigned total = 0;
        for (unsigned i = 0; i < iovcnt; ++i) { total += iov[i].iov_len; }
        unsigned numwritten = 0;
        while (numwritten < total) {
            unsigned num = sock.Writev(iov, iovcnt);
            numwritten += num;
            if (numwritten == total) break;
            while (iov->iov_len <= num) {
                num -= iov->iov_len;
                ++iov;
                --iovcnt;
            }
            if (num > 0) {
                // Finish writing this section...
                unsigned amount = num;
                while (amount < iov->iov_len) {
                    amount += sock.Write(((char*)iov->iov_base) + amount, iov->iov_len - amount);
                }
                ++iov;
                --iovcnt;
                numwritten += amount - num;
            }
        }
        ASSERT(total == numwritten);
    }

    void *RemoteQueue::FileThreadEntryPoint() {
        try {
            while (true) {
                try {
                    if (database->IsTerminated()) {
                        logger.Debug("Forced Shutdown (c: %llu)", clock.Get());
                        Shutdown();
                    }
                    {
                        AutoLock<QueueBase> al(*this);
                        if (dead) {
                            logger.Debug("Shutdown (c: %llu)", clock.Get());
                            break;
                        }
                    }
                    if (sock.Closed()) {
                        logger.Debug("Connecting (c: %llu)", clock.Get());
                        shared_ptr<Sync::Future<int> > conn;
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
                            actionThread->Start();
                        }
                    } else {
                        FileHandle *fds[2];
                        fds[0] = &sock;
                        fds[1] = holder->GetWakeup();
                        FileHandle::Poll(fds, fds+2, -1);
                        {
                            AutoLock<QueueBase> al(*this);
                            Read();
                            Signal();
                        }
                    }
                } catch (const ErrnoException &e) {
                    HandleError(e);
                }
            }
        } catch (const ShutdownException &e) {
            logger.Debug("Forced Shutdown (c: %llu)", clock.Get());
            // Do nothing, this just breaks us out of the loop
            // Can be thrown from ConnectWriter or ConnectReader
        } catch (...) {
            holder->CleanupQueue(GetKey());
            throw;
        }
        holder->CleanupQueue(GetKey());
        server->Wakeup();
        return 0;
    }

    void RemoteQueue::Signal() {
        AutoLock<QueueBase> al(*this);
        pendingAction = true;
        QueueBase::Signal();
    }

    void *RemoteQueue::ActionThreadEntryPoint() {
        AutoLock<QueueBase> al(*this);
        while (!dead) {
            InternalCheckStatus();
            while(!pendingAction && !dead) {
                Wait();
            }
            pendingAction = false;
        }
        return 0;
    }

    void RemoteQueue::InternalCheckStatus() {
        AutoLock<QueueBase> al(*this);
        if (sock.Closed()) {
            return;
        }

        clock.Tick();

        bool terminated = database->IsTerminated();
        if (sock.Eof() && !(readshutdown || writeshutdown)) {
            if (terminated) {
                if (!sock.Closed()) {
                    sock.Close();
                }
                Shutdown();
                return;
            }
            logger.Error("Eof detected but not shutdown! (c: %llu)", clock.Get());
            ASSERT(false, "EOF detected but not shutdown! (c: %llu)", clock.Get());
        }

        try {

            if (pendingGrow && !sentEnd) {
                SendGrowPacket();
                pendingGrow = false;
            }

            if (mode == WRITE) {
                if (!sentEnd) {
                    if (terminated) {
                        ThresholdQueue::ShutdownWriter();
                    }
                    // Write as much as we can
                    while (!queue->Empty() && (bytecount < readerlength)) {
                        SendEnqueuePacket();
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
                }
                if (writeshutdown) {
                    if (!sentEnd && (queue->Empty() || terminated)) {
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
                if (!sentEnd) {
                    if (terminated) {
                        ThresholdQueue::ShutdownReader();
                    }
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
            HandleError(e);
        }
    }

    void RemoteQueue::HandleError(const ErrnoException &e) {
        AutoLock<QueueBase> al(*this);
        if (readshutdown || writeshutdown) {
            Signal();
        }
        switch (e.Error()) {
        case EPIPE:
        case EBADF:
        case ECONNRESET:
            try {
                sock.Close();
            } catch (const ErrnoException &e) {}
            dead = true;
            break;
        default:
            logger.Error("Exception in RemoteQueue rethrowing (c: %llu e: %d): %s",
                    clock.Get(), e.Error(), e.what());
            throw;
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

    std::string RemoteQueue::GetState() {
        std::ostringstream oss;
        oss << std::boolalpha
            << "s: " << QueueLength() << ",mt: " << MaxThreshold() << ",c: " << Count()
            << ",f: " << Freespace() << ",rr: " << readrequest << ",wr: " << writerequest
            << ",te: " << NumEnqueued() << ",td: " << NumDequeued()
            << ",M: " << (mode == READ ? "r" : "w") << ",rl: " << readerlength
            << ",wl: " << writerlength << ",c: " << clock.Get() << ",bc: "
            << bytecount << ",pb: " << pendingBlock << ",se: " << sentEnd
            << ",pg: " << pendingGrow << ",pd4r: " << pendingD4RTag
            << ",d: " << dead;
        if (readshutdown) {
            oss << ",readshutdown";
        }
        if (writeshutdown) {
            oss << ",writeshutdown";
        }
        return oss.str();
    }

    void RemoteQueue::LogState() {
        ThresholdQueue::LogState();
        logger.Error("Mode: %s, Readerlength: %u, Writerlength %u, Clock: %llu, bytecount: %u",
                mode == READ ? "read" : "write", readerlength, writerlength, clock.Get(), bytecount);
        logger.Error("PendingBlock: %s, SentEnd: %s, PendingGrow: %s, PendingD4R: %s, Dead: %s",
                BoolString(pendingBlock), BoolString(sentEnd), BoolString(pendingGrow),
                BoolString(pendingD4RTag), BoolString(dead));
        logger.Error("FileThread id: %llu, Running: %s, ActionThread id: %llu, Running: %s",
                (unsigned long long)((pthread_t)(*fileThread)), BoolString(fileThread->Running()),
                (unsigned long long)((pthread_t)(*actionThread)), BoolString(actionThread->Running()));
        if (sock.Closed()) {
            logger.Error("Socket closed");
        }
    }
}

