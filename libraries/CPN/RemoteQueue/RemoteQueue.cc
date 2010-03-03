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
#include "QueueAttr.h"
#include "Database.h"
#include "ErrnoException.h"
#include "AutoLock.h"
#include <algorithm>
#include <sstream>

struct BoolGuard {
public:
    BoolGuard(bool &v) : val(v) { val = true; }
    ~BoolGuard() { val = false; }
private:
    bool &val;
};

namespace CPN {

    RemoteQueue::RemoteQueue(shared_ptr<Database> db, Mode_t mode_,
                ConnectionServer *s, const SimpleQueueAttr &attr)
        : ThresholdQueue(db, attr, QueueLength(attr.GetLength(), attr.GetAlpha(), mode_)),
        mode(mode_),
        alpha(attr.GetAlpha()),
        server(s),
        mocknode(mode_ == READ ? attr.GetReaderNodeKey() : attr.GetWriterNodeKey()),
        readerlength(QueueLength(attr.GetLength(), attr.GetAlpha(), READ)),
        writerlength(QueueLength(attr.GetLength(), attr.GetAlpha(), WRITE)),
        bytecount(0),
        pendingBlock(false),
        sentEnd(false),
        pendingGrow(false),
        pendingD4RTag(false),
        incheckstatus(false)
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
        Start();
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

    unsigned RemoteQueue::Freespace() const {
        Sync::AutoLock<const QueueBase> al(*this);
        if (mode == READ) {
            return queue->Freespace();
        } else {
            return (readerlength + writerlength) - (queue->Count() + bytecount);
        }
    }

    bool RemoteQueue::Full() const {
        Sync::AutoLock<const QueueBase> al(*this);
        if (mode == READ) {
            return queue->Full();
        } else {
            return (readerlength + writerlength) <= (queue->Count() + bytecount);
        }
    }

    unsigned RemoteQueue::QueueLength() const {
        Sync::AutoLock<const QueueBase> al(*this);
        return readerlength + writerlength;
    }

    void RemoteQueue::Grow(unsigned queueLen, unsigned maxThresh) {
        Sync::AutoLock<QueueBase> al(*this);
        readerlength = QueueLength(queueLen, alpha, READ);
        writerlength = QueueLength(queueLen, alpha, WRITE);
        const unsigned newlen = (mode == WRITE ? writerlength : readerlength);
        ThresholdQueue::Grow(newlen, maxThresh);
        pendingGrow = true;
        InternalCheckStatus();
    }

    void RemoteQueue::ShutdownReader() {
        ThresholdQueue::ShutdownReader();
        InternalCheckStatus();
    }

    void RemoteQueue::ShutdownWriter() {
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
        ThresholdQueue::InternalDequeue(count);
        InternalCheckStatus();
    }

    void RemoteQueue::InternalEnqueue(unsigned count) {
        ThresholdQueue::InternalEnqueue(count);
        InternalCheckStatus();
    }

    void RemoteQueue::SignalReaderTagChanged() {
        Sync::AutoLock<QueueBase> al(*this);
        pendingD4RTag = true;
        ThresholdQueue::SignalReaderTagChanged();
        InternalCheckStatus();
    }

    void RemoteQueue::SignalWriterTagChanged() {
        Sync::AutoLock<QueueBase> al(*this);
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
        unsigned count = queue->Count();
        const unsigned maxthresh = queue->MaxThreshold();
        const unsigned expectedfree = readerlength - bytecount;
        if (count > maxthresh) { count = maxthresh; }
        if (count > expectedfree) { count = expectedfree; }
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
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        readrequest = 0;
        bytecount -= packet.Count();
    }

    void RemoteQueue::SendDequeuePacket() {
        Packet packet(PACKET_DEQUEUE);
        SetupPacket(packet);
        const unsigned count = bytecount - queue->Count();
        packet.Count(count);
        PacketEncoder::SendPacket(packet);
        bytecount -= count;
    }

    void RemoteQueue::ReadBlockPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        readrequest = packet.Requested();
        if (useD4R) {
            pendingD4RTag = true;
        }
    }

    void RemoteQueue::SendReadBlockPacket() {
        Packet packet(PACKET_READBLOCK);
        SetupPacket(packet);
        packet.Requested(readrequest);
        PacketEncoder::SendPacket(packet);
    }

    void RemoteQueue::WriteBlockPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        ASSERT(mode == READ);
        ASSERT(!writeshutdown);
        writerequest = packet.Requested();
        if (useD4R) {
            pendingD4RTag = true;
        }
    }

    void RemoteQueue::SendWriteBlockPacket() {
        Packet packet(PACKET_WRITEBLOCK);
        SetupPacket(packet);
        packet.Requested(writerequest);
        PacketEncoder::SendPacket(packet);
    }

    void RemoteQueue::EndOfWritePacket(const Packet &packet) {
        clock.Update(packet.Clock());
        ASSERT(mode == READ);
        ASSERT(!writeshutdown);
        QueueBase::ShutdownWriter();
    }

    void RemoteQueue::SendEndOfWritePacket() {
        Packet packet(PACKET_ENDOFWRITE);
        SetupPacket(packet);
        PacketEncoder::SendPacket(packet);
        sock.ShutdownWrite();
    }

    void RemoteQueue::EndOfReadPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        QueueBase::ShutdownReader();
    }

    void RemoteQueue::SendEndOfReadPacket() {
        Packet packet(PACKET_ENDOFREAD);
        SetupPacket(packet);
        PacketEncoder::SendPacket(packet);
        sock.ShutdownWrite();
    }

    void RemoteQueue::GrowPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        const unsigned queueLen = packet.QueueSize();
        readerlength = QueueLength(queueLen, alpha, READ);
        writerlength = QueueLength(queueLen, alpha, WRITE);
        const unsigned newlen = (mode == WRITE ? writerlength : readerlength);
        ThresholdQueue::Grow(newlen, packet.MaxThreshold());
    }

    void RemoteQueue::SendGrowPacket() {
        Packet packet(PACKET_GROW);
        SetupPacket(packet);
        packet.QueueSize(readerlength + writerlength);
        packet.MaxThreshold(queue->MaxThreshold());
        PacketEncoder::SendPacket(packet);
    }

    void RemoteQueue::D4RTagPacket(const Packet &packet) {
        clock.Update(packet.Clock());
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
        while (true) {
            try {
                if (sock.Closed()) {
                    if (IsReaderShutdown() || IsWriterShutdown() || database->IsTerminated()) {
                        return 0;
                    }
                    shared_ptr<Future<int> > conn;
                    if (mode == WRITE) {
                        conn = server->ConnectWriter(GetWriterKey());
                    } else {
                        conn = server->ConnectReader(GetReaderKey());
                    }
                    if (conn) {
                        sock.Reset();
                        sock.FD(conn->Get());
                    }
                } else {
                    sock.Poll(-1);
                    InternalCheckStatus();
                }
            } catch (const ErrnoException &e) {
                logger.Error("Exception in RemoteQueue main loop (c: %llu e: %d): %s",
                        clock.Get(), e.Error(), e.what());
            }
        }
    }

    void RemoteQueue::InternalCheckStatus() {
        Sync::AutoLock<QueueBase> al(*this);
        if (incheckstatus) { return; }
        BoolGuard guard(incheckstatus);
        clock.Tick();

        if (sock.Eof() && !(readshutdown || writeshutdown)) {
            logger.Error("Eof detected but not shutdown! (c: %llu)", clock.Get());
            ASSERT(false, "EOF detected but not shutdown! (c: %llu)", clock.Get());
        }

        Read();

        if (pendingGrow) {
            SendGrowPacket();
            pendingGrow = false;
        }

        if (mode == WRITE) {
            // If we can write more try to write more
            if (bytecount < readerlength) {
                SendEnqueuePacket();
            }
            // If we can still write more, have the next call to Poll
            // check writeability
            if (bytecount < readerlength) {
                sock.Writeable(false);
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
                sock.Close();
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
                sock.Close();
            }
        }
    }

    unsigned RemoteQueue::QueueLength(unsigned length, double alpha, Mode_t mode) {
        unsigned writerlen = unsigned(((double)length)*alpha);
        if (mode == READ) {
            return std::min<unsigned>(length - writerlen, 1);
        } else {
            return std::min<unsigned>(length, 1);
        }
    }
}
