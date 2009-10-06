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

#include "StreamEndpoint.h"
#include "QueueBase.h"

namespace CPN {

    StreamEndpoint::StreamEndpoint(Key_t rkey, Key_t wkey, Mode_t m)
        : writecount(0),
        readcount(0),
        readerkey(rkey),
        writerkey(wkey),
        mode(m),
        shuttingdown(false)
    {
        PacketDecoder::Enable(false);
    }

    void StreamEndpoint::RMHEnqueue(Key_t wkey, Key_t rkey) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(!shuttingdown, "Enqueue on a shutdown queue!");
        ASSERT(wkey == writerkey && rkey == readerkey);
        ASSERT(mode == WRITE);
        CheckBlockedEnqueues();
        WriteSome();
    }


    void StreamEndpoint::WMHDequeue(Key_t rkey, Key_t wkey) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(rkey == readerkey && wkey == writerkey);
        ASSERT(mode == READ);
        unsigned numchans = queue->NumChannels();
        // Difference between how many bytes we think are in the
        // queue and how many are actually there
        unsigned length = readcount - queue->Count();
        encoder.SendDequeue(length, numchans);
        readcount -= length;
        WriteSome();
    }

    void StreamEndpoint::WMHReadBlock(Key_t rkey, Key_t wkey, unsigned requested) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(!shuttingdown, "Block on a shutdown queue!");
        ASSERT(rkey == readerkey && wkey == writerkey);
        ASSERT(mode == READ);
        encoder.SendReadBlock(requested);
        WriteSome();
    }

    void StreamEndpoint::RMHWriteBlock(Key_t wkey, Key_t rkey, unsigned requested) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(!shuttingdown, "Block on a shutdown queue!");
        ASSERT(wkey == writerkey && rkey == readerkey);
        ASSERT(mode == WRITE);
        CheckBlockedEnqueues();
        encoder.SendWriteBlock(requested);
        WriteSome();
    }

    void StreamEndpoint::RMHEndOfWriteQueue(Key_t wkey, Key_t rkey) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(wkey == writerkey && rkey == readerkey);
        ASSERT(mode == WRITE);
        // send the message on and start our shutdown
        shuttingdown = true;
        encoder.SendEndOfWriteQueue();
        WriteSome();
    }

    void StreamEndpoint::WMHEndOfReadQueue(Key_t rkey, Key_t wkey) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(rkey == readerkey && wkey == writerkey);
        ASSERT(mode == READ);
        // send the message on and start our shutdown
        shuttingdown = true;
        encoder.SendEndOfReadQueue();
        WriteSome();
    }

    void StreamEndpoint::RMHTagChange(Key_t wkey, Key_t rkey) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(wkey == writerkey && rkey == readerkey);
        ASSERT(mode == WRITE);
        ASSERT(false, "Unimplemented");
    }

    void StreamEndpoint::WMHTagChange(Key_t rkey, Key_t wkey) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(rkey == readerkey && wkey == writerkey);
        ASSERT(mode == READ);
        ASSERT(false, "Unimplemented");
    }

    bool StreamEndpoint::ReadReady() {
        Sync::AutoReentrantLock arl(*lock);
        return PacketDecoder::Enabled();
    }

    bool StreamEndpoint::WriteReady() {
        Sync::AutoReentrantLock arl(*lock);
        if (!encoder.BytesReady()) {
            if (mode == WRITE) {
                if (!EnqueueBlocked()) {
                    return WriteEnqueue();
                }
            }
            return false;
        }
        return true;
    }

    void StreamEndpoint::ReadSome() {
        Sync::AutoReentrantLock arl(*lock);
        Async::Stream stream(descriptor);
        unsigned numtoread = 0;
        unsigned numread = 0;
        bool loop = true;
        while (loop) {
            void *ptr = PacketDecoder::GetDecoderBytes(numtoread);
            numread = stream.Read(ptr, numtoread);
            if (0 == numread) {
                if  (!stream) {
                    // The other end closed the connection!!
                    stream.Close();
                }
                loop = false;
            } else {
                PacketDecoder::ReleaseDecoderBytes(numread);
            }
        }
    }

    void StreamEndpoint::WriteSome() {
        Sync::AutoReentrantLock arl(*lock);
        Async::Stream stream(descriptor);
        while (true) {
            unsigned towrite = 0;
            const void *ptr = encoder.GetEncodedBytes(towrite);
            if (0 == towrite) {
                if (mode == WRITE) {
                    if (!EnqueueBlocked()) {
                        if (WriteEnqueue()) {
                            continue;
                        } else if (queue->Empty()
                                && !encoder.BytesReady()
                                && shuttingdown) {
                            // Nothing left to write
                            stream.Close();
                        }
                    }
                }
                break;
            }
            unsigned numwritten = stream.Write(ptr, towrite);
            if (numwritten == 0) {
                break;
            } else {
                encoder.ReleaseEncodedBytes(numwritten);
            }
        }
    }

    // All of these Receive functions will end up beign called in the
    // context of ReadSome.

    void StreamEndpoint::ReceivedEnqueue(void *data, unsigned length, unsigned numchannels) {
        ASSERT(numchannels == queue->NumChannels(),
                "Endpoints disagree on number of channels.");
        ASSERT(mode == READ);
        // We want to send the enqueue message after we have actually enqueued the data
        ENSURE(queue->RawEnqueue(data, length, numchannels, length),
                "Enqueue throttling over the socket failed.");
        readcount += length;
        rmh->RMHEnqueue(writerkey, readerkey);
    }

    void StreamEndpoint::ReceivedDequeue(unsigned length, unsigned numchannels) {
        ASSERT(numchannels == queue->NumChannels(),
                "Endpoints disagree on number of channels.");
        ASSERT(mode == WRITE);
        writecount -= length;
        CheckBlockedEnqueues();
        wmh->WMHDequeue(readerkey, writerkey);
    }

    void StreamEndpoint::ReceivedReadBlock(unsigned requested) {
        ASSERT(mode == WRITE);
        wmh->WMHReadBlock(readerkey, writerkey, requested);
    }

    void StreamEndpoint::ReceivedWriteBlock(unsigned requested) {
        ASSERT(mode == READ);
        rmh->RMHWriteBlock(writerkey, readerkey, requested);
    }

    void StreamEndpoint::ReceiveEndOfWriteQueue() {
        ASSERT(mode == READ);
        shuttingdown = true;
        rmh->RMHEndOfWriteQueue(writerkey, readerkey);
    }

    void StreamEndpoint::ReceiveEndOfReadQueue() {
        ASSERT(mode == WRITE);
        shuttingdown = true;
        wmh->WMHEndOfReadQueue(readerkey, writerkey);
    }

    void StreamEndpoint::CheckBlockedEnqueues() {
        while (!EnqueueBlocked() && WriteEnqueue()) {
            // Would it be a good idea to call WriteSome here?
        }
    }

    bool StreamEndpoint::WriteEnqueue() {
        ASSERT(mode == WRITE);
        // Send as much as we can, but no more than maxthreshold
        unsigned maxthresh = queue->MaxThreshold();
        unsigned amount = queue->Count();
        if (amount > maxthresh) { amount = maxthresh; }
        if (amount == 0) { return false; }
        unsigned numchans = queue->NumChannels();
        std::vector<const void*> data(numchans, 0);
        for (unsigned chan = 0; chan < numchans; ++chan) {
            data[chan] = queue->GetRawDequeuePtr(amount, chan);
            ASSERT(data[chan], "No data in the queue, inconsistant queue state.");
        }
        encoder.SendEnqueue(&data[0], amount, numchans);
        queue->Dequeue(amount);
        writecount += amount;
        return true;
    }

    bool StreamEndpoint::EnqueueBlocked() {
        return 2*writecount >= queue->QueueLength();
    }

    void StreamEndpoint::SetDescriptor(Async::DescriptorPtr desc) {
        descriptor = desc;
        descriptor->ConnectReadable(sigc::mem_fun(this, &StreamEndpoint::ReadReady));
        descriptor->ConnectWriteable(sigc::mem_fun(this, &StreamEndpoint::WriteReady));
        descriptor->ConnectOnRead(sigc::mem_fun(this, &StreamEndpoint::ReadSome));
        descriptor->ConnectOnWrite(sigc::mem_fun(this, &StreamEndpoint::WriteSome));
    }

    void StreamEndpoint::SetQueue(shared_ptr<QueueBase> q) {
        lock = &q->GetLock();
        queue = q;
        rmh = queue->GetReaderMessageHandler();
        wmh = queue->GetWriterMessageHandler();
        PacketDecoder::Enable(true);
    }
}

