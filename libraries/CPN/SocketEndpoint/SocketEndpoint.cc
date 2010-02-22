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

#include "SocketEndpoint.h"
#include "QueueAttr.h"
#include "Assert.h"
#include "ToString.h"
#include "ErrnoException.h"

#if 0
#define FUNC_TRACE logger.Trace("%s (c: %llu)", __PRETTY_FUNCTION__, clock.Get())
#else
#define FUNC_TRACE
#endif

struct BoolGuard {
public:
    BoolGuard(bool &v) : val(v) { val = true; }
    ~BoolGuard() { val = false; }
private:
    bool &val;
};

namespace CPN {

    SocketEndpoint::SocketEndpoint(shared_ptr<Database> db, Mode_t mode_,
            KernelBase *kmh_, const SimpleQueueAttr &attr)
        : ThresholdQueue(db, attr),
        mocknode(mode_ == READ ? attr.GetReaderNodeKey() : attr.GetWriterNodeKey()),
        logger(kmh_->GetLogger(), Logger::INFO),
        status(LIVE),
        mode(mode_),
        kmh(kmh_),
        writecount(0),
        readcount(0),
        pendingDequeue(false),
        pendingBlock(false),
        sentEnd(false),
        pendingGrow(false),
        inread(false),
        incheckstatus(false)
    {
        Writeable(false);
        if (mode == READ) {
            SetWriterNode(&mocknode);
        } else {
            SetReaderNode(&mocknode);
        }
        logger.Name(ToString("SocketEndpoint(m:%s, r:%llu, w: %llu)",
                    mode == READ ? "r" : "w", readerkey, writerkey));
    }

    SocketEndpoint::~SocketEndpoint() {
    }

    SocketEndpoint::Status_t SocketEndpoint::GetStatus() const {
        Sync::AutoLock<const QueueBase> arl(*this);
        return status;
    }

    double SocketEndpoint::CheckStatus() {
        Sync::AutoLock<QueueBase> arl(*this);
        InternCheckStatus();

        // this is arbitrary
        if (status == LIVE) {
            if (Closed()) {
                // 30s
                return 30;
            }
        } else if (status == DIEING) {
            return 30;
        }
        return -1;
    }

    void SocketEndpoint::InternalDequeue(unsigned count) {
        ThresholdQueue::InternalDequeue(count);
        InternCheckStatus();
    }

    void SocketEndpoint::InternalEnqueue(unsigned count) {
        ThresholdQueue::InternalEnqueue(count);
        InternCheckStatus();
    }

    void SocketEndpoint::Grow(unsigned queueLen, unsigned maxThresh) {
        Sync::AutoLock<QueueBase> arl(*this);
        ThresholdQueue::Grow(queueLen, maxThresh);
        NotifyFreespace();
        pendingGrow = true;
        InternCheckStatus();
    }

    void SocketEndpoint::WaitForFreespace() {
        Sync::AutoLock<QueueBase> arl(*this);
        ASSERT(mode == WRITE);
        pendingBlock = true;
        InternCheckStatus();
        QueueBase::WaitForFreespace();
    }

    void SocketEndpoint::WaitForData() {
        Sync::AutoLock<QueueBase> arl(*this);
        ASSERT(mode == READ);
        pendingBlock = true;
        InternCheckStatus();
        QueueBase::WaitForData();
    }

    void SocketEndpoint::ShutdownReader() {
        Sync::AutoLock<QueueBase> al(*this);
        QueueBase::ShutdownReader();
        InternCheckStatus();
    }

    void SocketEndpoint::ShutdownWriter() {
        Sync::AutoLock<QueueBase> al(*this);
        QueueBase::ShutdownWriter();
        InternCheckStatus();
    }

    void SocketEndpoint::SignalWriterTagChanged() {
        Sync::AutoLock<QueueBase> al(*this);
        pendingD4RTag = true;
        QueueBase::SignalWriterTagChanged();
        InternCheckStatus();
    }

    void SocketEndpoint::SignalReaderTagChanged() {
        Sync::AutoLock<QueueBase> al(*this);
        pendingD4RTag = true;
        QueueBase::SignalReaderTagChanged();
        InternCheckStatus();
    }

    void SocketEndpoint::OnRead() {
        Sync::AutoLock<QueueBase> arl(*this);
        // Don't allow recursive calls to OnRead
        // Can happen because *Packet functions call
        // InternCheckStatus
        if (inread) { return; }
        BoolGuard guard(inread);

        try {
            while (Good()) {
                unsigned numtoread = 0;
                void *ptr = PacketDecoder::GetDecoderBytes(numtoread);
                unsigned numread = Recv(ptr, numtoread, false);
                if (numread == 0) {
                    if (Eof()) {
                        // Eof
                        logger.Debug("Read EOF");
                        //ASSERT(readshutdown || writeshutdown, "Read EOF before shutdown");
                    }
                    break;
                } else {
                    PacketDecoder::ReleaseDecoderBytes(numread);
                }
            }
        } catch (const ErrnoException &e) {
            logger.Error("Exception on read (c: %llu e: %d): %s", clock.Get(), e.Error(), e.what());
        }
    }

    void SocketEndpoint::OnWrite() {
        Sync::AutoLock<QueueBase> arl(*this);
        logger.Error("%s (c: %llu)", __PRETTY_FUNCTION__, clock.Get());
    }

    void SocketEndpoint::OnError() {
        Sync::AutoLock<QueueBase> arl(*this);
        // Error on socket.
        logger.Error("%s (c: %llu)", __PRETTY_FUNCTION__, clock.Get());
    }

    void SocketEndpoint::OnHup() {
        Sync::AutoLock<QueueBase> arl(*this);
        // If I understand correctly this will be called if
        // poll detects that if we try to write we would get EPIPE
        // This only seems to be the case on linux systems, others
        // will call this when a read would give eof as well
        logger.Debug("Hangup received (c: %llu)", clock.Get());
    }

    void SocketEndpoint::OnInval() {
        Sync::AutoLock<QueueBase> arl(*this);
        // Our file descriptor is invalid
        logger.Debug("%s (c: %llu)", __PRETTY_FUNCTION__, clock.Get());
        ASSERT(Closed());
    }

    bool SocketEndpoint::Readable() const {
        Sync::AutoLock<const QueueBase> arl(*this);
        return Good();
    }

    void SocketEndpoint::EnqueuePacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE;
        ASSERT( (packet.Mode() == WRITE) &&
        (packet.SourceKey() == writerkey) &&
        (packet.DestinationKey() == readerkey)
        );
        ASSERT(mode == READ);
        ASSERT(!writeshutdown);
        unsigned count = packet.Count();
        if (count > queue->Freespace() || count > queue->MaxThreshold()) {
            ThresholdQueue::Grow(queue->Count() + count, count);
            pendingGrow = true;
        }
        std::vector<iovec> iovs;
        for (unsigned i = 0; i < packet.NumChannels(); ++i) {
            iovec iov;
            iov.iov_base = queue->GetRawEnqueuePtr(count, i);
            ASSERT(iov.iov_base, "Internal throttle failed! (c: %llu)", clock.Get());
            iov.iov_len = count;
            iovs.push_back(iov);
        }
        unsigned numread = Readv(&iovs[0], iovs.size());
        // Need to figure out what to do when this fails.
        ASSERT(numread == packet.DataLength());
        queue->Enqueue(count);
        readcount += count;
        writerequest = 0;
        NotifyData();
    }

    void SocketEndpoint::DequeuePacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE;
        ASSERT( (packet.Mode() == READ) &&
        (packet.SourceKey() == readerkey) &&
        (packet.DestinationKey() == writerkey)
        );
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        readrequest = 0;
        writecount -= packet.Count();
        CheckEnqueue();
        InternCheckStatus();
    }

    void SocketEndpoint::ReadBlockPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE;
        ASSERT( (packet.Mode() == READ) &&
        (packet.SourceKey() == readerkey) &&
        (packet.DestinationKey() == writerkey)
        );
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        readrequest = packet.Requested();
        if (useD4R) {
            pendingD4RTag = true;
        }
        CheckEnqueue();
        InternCheckStatus();
    }

    void SocketEndpoint::WriteBlockPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE;
        ASSERT( (packet.Mode() == WRITE) &&
        (packet.SourceKey() == writerkey) &&
        (packet.DestinationKey() == readerkey)
        );
        ASSERT(mode == READ);
        ASSERT(!writeshutdown);
        writerequest = packet.Requested();
        if (useD4R) {
            pendingD4RTag = true;
        }
        InternCheckStatus();
    }

    void SocketEndpoint::EndOfWritePacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE;
        ASSERT( (packet.Mode() == WRITE) &&
        (packet.SourceKey() == writerkey) &&
        (packet.DestinationKey() == readerkey)
        );
        ASSERT(mode == READ);
        ASSERT(!writeshutdown);
        QueueBase::ShutdownWriter();
        InternCheckStatus();
    }

    void SocketEndpoint::EndOfReadPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE;
        ASSERT( (packet.Mode() == READ) &&
        (packet.SourceKey() == readerkey) &&
        (packet.DestinationKey() == writerkey)
        );
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        QueueBase::ShutdownReader();
        InternCheckStatus();
    }

    void SocketEndpoint::GrowPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE;
        ThresholdQueue::Grow(packet.QueueSize(), packet.MaxThreshold());
        NotifyFreespace();
    }

    void SocketEndpoint::D4RTagPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE;
        ASSERT(packet.DataLength() == sizeof(D4R::Tag));
        D4R::Tag tag;
        unsigned numread = Read(&tag, sizeof(tag));
        ASSERT(numread == sizeof(tag));
        mocknode.SetPublicTag(tag);
        if (mode == WRITE) {
            QueueBase::SignalReaderTagChanged();
        } else {
            QueueBase::SignalWriterTagChanged();
        }
    }

    void SocketEndpoint::IDReaderPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE;
        ASSERT(false, "Shouldn't receive an ID packet once the connection is successful");
    }

    void SocketEndpoint::IDWriterPacket(const Packet &packet) {
        clock.Update(packet.Clock());
        FUNC_TRACE;
        ASSERT(false, "Shouldn't receive an ID packet once the connection is successful");
    }

    void SocketEndpoint::WriteBytes(const iovec *iov, unsigned iovcnt) {
        int numwritten = Writev(iov, iovcnt);
#ifndef NDEBUG
        int total = 0;
        for (unsigned i = 0; i < iovcnt; ++i) {
            total += iov[i].iov_len;
        }
        ASSERT(numwritten == total, "Writev did not completely write data. (c: %llu)", clock.Get());
#endif
    }

    void SocketEndpoint::InternCheckStatus() {
        // return if recursive call
        if (incheckstatus) { return; }
        BoolGuard guard(incheckstatus);
        if (status == DEAD) { return; }
        clock.Tick();

        try {

            if (Eof() && !(readshutdown || writeshutdown)) {
                logger.Error("Eof detected but not shutdown! (c: %llu)", clock.Get());
                ASSERT(false, "EOF detected but not shutdown! (c: %llu)", clock.Get());
            }

            if (Closed()) {
                if (status == DIEING) {
                    status = DEAD;
                    return;
                }
                if (status == LIVE) {
                    while (true) {
                        if (connection) {
                            if (connection->Done()) {
                                ASSERT(Closed());
                                FileHandler::Reset();
                                FileHandler::FD(connection->Get());
                                connection.reset();
                                logger.Debug("Connection established (c: %llu)", clock.Get());
                            } else { return; }
                        } else if (Closed()) {
                            logger.Debug("Getting new connection (c: %llu)", clock.Get());
                            if (mode == READ) {
                                connection = kmh->GetReaderDescriptor(readerkey, writerkey);
                            } else {
                                connection = kmh->GetWriterDescriptor(readerkey, writerkey);
                            }
                        } else { break; }
                    }
                }
            }

            if (pendingGrow) {
                SendGrow();
            }

            if (mode == READ) {
                // In read mode
     
                CheckDequeue();

                if (pendingBlock) {
                    OnRead();
                    if (readrequest > queue->Count()) {
                        SendReadBlock();
                    } else {
                        pendingBlock = false;
                    }
                }

                if (pendingD4RTag) {
                    if (reader && !readshutdown) {
                        SendD4RTag();
                    }
                }

                // If we have a read shutdown we need to send the 
                // end of read queue message and shutdown the write
                // side of the socket.
                if (readshutdown) {
                    if (!sentEnd) { SendEndOfRead(); }
                }

                // if we have a write shutdown we can go ahead and shutdown
                // the write side of our socket and send a end of reader
                // down the socket if not already sent and start actually
                // shutting down
                if (writeshutdown) {
                    if (!sentEnd) { SendEndOfRead(); }
                    status = DEAD;
                    logger.Debug("Closing connection (c: %llu)", clock.Get());
                    Close();
                }

            } else {
                ASSERT(mode == WRITE);

                // In write mode
                CheckEnqueue();

                if (pendingBlock) {
                    OnRead();
                    if (writerequest > queue->Freespace()) {
                        SendWriteBlock();
                    } else {
                        pendingBlock = false;
                    }
                }

                if (pendingD4RTag) {
                    if (writer && !writeshutdown) {
                        SendD4RTag();
                    }
                }

                // if we have a read shutdown we ensure we have written
                // a writer end and die. There will be no more enqueues
                // and there is no more data to be read.
                if (readshutdown) {
                    if (!sentEnd) { SendEndOfWrite(); }
                    status = DEAD;
                    logger.Debug("Closing connection (c: %llu)", clock.Get());
                    Close();
                }

                // If we have a write shutdown we need to finish sending any data we
                // have and then send across the writer end and wait for the confirming
                // reader end
                if (writeshutdown) {
                    if ( queue->Empty() && !sentEnd ) {
                        SendEndOfWrite();
                    }
                }
            }
        } catch (const ErrnoException &e) {
            logger.Error("Exception (c: %llu, e: %d): %s", clock.Get(), e.Error(), e.what());
        }
    }

    void SocketEndpoint::CheckDequeue() {
        if (pendingDequeue || readcount - queue->Count() > 0) {
            SendDequeue();
        }
    }

    void SocketEndpoint::CheckEnqueue() {
        while (!queue->Empty() && !EnqueueBlocked()) {
            SendEnqueue();
        }
    }

    bool SocketEndpoint::EnqueueBlocked() {
        return (2 * writecount) > queue->QueueLength();
    }

    void SocketEndpoint::SetupPacketDefaults(Packet &packet) {
        if (mode == WRITE) { packet.SourceKey(writerkey).DestinationKey(readerkey); }
        else { packet.SourceKey(readerkey).DestinationKey(writerkey); }
        packet.Mode(mode).Status(status).NumChannels(queue->NumChannels());
        packet.Clock(clock.Tick());
    }

    void SocketEndpoint::SendEnqueue() {
        FUNC_TRACE;
        ASSERT(!Closed());
        unsigned count = queue->Count();
        const unsigned maxthresh = queue->MaxThreshold();
        const unsigned expectedfree = queue->QueueLength() - writecount;
        if (count > maxthresh) { count = maxthresh; }
        if (count > expectedfree) { count = expectedfree; }
        unsigned datalength = count * queue->NumChannels();
        Packet packet(datalength, PACKET_ENQUEUE);
        SetupPacketDefaults(packet);
        packet.Count(count).BytesQueued(queue->Count());
        PacketEncoder::SendEnqueue(packet, *queue);
        writecount += count;
        QueueBase::NotifyFreespace();
    }

    void SocketEndpoint::SendWriteBlock() {
        FUNC_TRACE;
        ASSERT(!Closed());
        Packet packet(PACKET_WRITEBLOCK);
        SetupPacketDefaults(packet);
        packet.Requested(writerequest);
        PacketEncoder::SendPacket(packet);
        pendingBlock = false;
    }

    void SocketEndpoint::SendEndOfWrite() {
        FUNC_TRACE;
        ASSERT(!Closed());
        Packet packet(PACKET_ENDOFWRITE);
        SetupPacketDefaults(packet);
        PacketEncoder::SendPacket(packet);
        sentEnd = true;
        SockHandler::ShutdownWrite();
    }

    void SocketEndpoint::SendDequeue() {
        FUNC_TRACE;
        ASSERT(!Closed());
        Packet packet(PACKET_DEQUEUE);
        SetupPacketDefaults(packet);
        unsigned count = readcount - queue->Count();
        packet.Count(count);
        PacketEncoder::SendPacket(packet);
        readcount -= count;
        pendingDequeue = false;
    }

    void SocketEndpoint::SendReadBlock() {
        FUNC_TRACE;
        ASSERT(!Closed());
        Packet packet(PACKET_READBLOCK);
        SetupPacketDefaults(packet);
        packet.Requested(readrequest);
        PacketEncoder::SendPacket(packet);
        pendingBlock = false;
    }

    void SocketEndpoint::SendEndOfRead() {
        FUNC_TRACE;
        ASSERT(!Closed());
        Packet packet(PACKET_ENDOFREAD);
        SetupPacketDefaults(packet);
        PacketEncoder::SendPacket(packet);
        sentEnd = true;
        SockHandler::ShutdownWrite();
    }

    void SocketEndpoint::SendGrow() {
        FUNC_TRACE;
        ASSERT(!Closed());
        Packet packet(PACKET_GROW);
        SetupPacketDefaults(packet);
        packet.QueueSize(queue->QueueLength());
        packet.MaxThreshold(queue->MaxThreshold());
        PacketEncoder::SendPacket(packet);
        pendingGrow = false;
    }

    void SocketEndpoint::SendD4RTag() {
        FUNC_TRACE;
        ASSERT(!Closed());
        Packet packet(PACKET_D4RTAG);
        SetupPacketDefaults(packet);
        packet.QueueSize(queue->QueueLength());
        packet.MaxThreshold(queue->MaxThreshold());
        packet.DataLength(sizeof(D4R::Tag));
        D4R::Tag tag;
        if (mode == READ) {
            tag = reader->GetPublicTag();
        } else {
            tag = writer->GetPublicTag();
        }
        PacketEncoder::SendPacket(packet, &tag);
        pendingD4RTag = false;
    }

    void SocketEndpoint::LogState() {
        logger.Debug("Printing state (w:%llu r:%llu c:%llu)", readerkey, writerkey, clock.Get());
        switch (status) {
            case LIVE:
                logger.Debug("State live");
                break;
            case DEAD:
                logger.Debug("State dead");
                break;
            case DIEING:
                logger.Debug("State dieing");
                break;
            default:
                ASSERT(false);
        }
        if (Closed()) {
            logger.Debug("Not Connected");
            if (connection) {
                logger.Debug("Pending connection (%p)", connection.get());
                if (connection->Done()) {
                    logger.Debug("Pending done");
                }
            }
        } else {
            logger.Debug("Connected");
            if (Eof()) {
                logger.Debug("Read eof");
            }
        }
        if (mode == READ) {
            logger.Debug("Readcount %u (in queue %u)", readcount, queue->Count());
            if (pendingDequeue) {
                logger.Debug("Dequeue pending");
            }
            if (sentEnd) {
                logger.Debug("End of read sent.");
            }
        } else {
            logger.Debug("Writecount %u", writecount);
            if (!queue->Empty()) {
                logger.Debug("Enqueue pending (count %u)", queue->Count());
                if (EnqueueBlocked()) {
                    logger.Debug("Enqueue blocked");
                }
            }
            if (sentEnd) {
                logger.Debug("End of write sent.");
            }
        }
        if (pendingBlock) {
            if (mode == READ) {
                logger.Debug("Pending block request %u", readrequest);
            } else {
                logger.Debug("Pending block request %u", writerequest);
            }
        }
        if (readshutdown) {
            logger.Debug("Reader shutdown");
        }
        if (writeshutdown) {
            logger.Debug("Writer shutdown");
        }
        if (pendingGrow) {
            logger.Debug("Pending grow (q: %u, t: %u)", queue->QueueLength(), queue->MaxThreshold());
        }
        if (pendingD4RTag) {
            logger.Debug("Pending send of D4R tag");
        }
    }
}

