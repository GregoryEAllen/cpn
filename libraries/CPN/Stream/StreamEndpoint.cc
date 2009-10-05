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

    StreamEndpoint::StreamEndpoint(Key_t rkey, Key_t wkey)
        : writecount(0),
        readcount(0),
        readerkey(rkey),
        writerkey(wkey),
        shuttingdown(false)
    {
        PacketDecoder::Enable(false);
    }

    void StreamEndpoint::RMHEnqueue(Key_t src, Key_t dst) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(src == writerkey && dst == readerkey);
        CheckBlockedEnqueues();
        WriteSome();
    }


    void StreamEndpoint::WMHDequeue(Key_t src, Key_t dst) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(src == readerkey && dst == writerkey);
        unsigned numchans = queue->NumChannels();
        // Difference between how many bytes we think are in the
        // queue and how many are actually there
        unsigned length = readcount - queue->Count();
        encoder.SendDequeue(length, numchans);
        readcount -= length;
        WriteSome();
    }

    void StreamEndpoint::WMHReadBlock(Key_t src, Key_t dst) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(src == readerkey && dst == writerkey);
        // dummy value for now
        encoder.SendReadBlock(0);
        WriteSome();
    }

    void StreamEndpoint::RMHWriteBlock(Key_t src, Key_t dst) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(src == writerkey && dst == readerkey);
        CheckBlockedEnqueues();
        // dummy value for now
        encoder.SendWriteBlock(0);
        WriteSome();
    }

    void StreamEndpoint::RMHEndOfWriteQueue(Key_t src, Key_t dst) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(src == writerkey && dst == readerkey);
        // TODO send the message on and start our shutdown
        shuttingdown = true;
        ASSERT(false, "Unimplemented");
    }

    void StreamEndpoint::WMHEndOfReadQueue(Key_t src, Key_t dst) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(src == readerkey && dst == writerkey);
        // TODO send the message on and start our shutdown
        shuttingdown = true;
        ASSERT(false, "Unimplemented");
    }

    void StreamEndpoint::RMHTagChange(Key_t src, Key_t dst) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(src == writerkey && dst == readerkey);
        ASSERT(false, "Unimplemented");
    }

    void StreamEndpoint::WMHTagChange(Key_t src, Key_t dst) {
        Sync::AutoReentrantLock arl(*lock);
        ASSERT(src == readerkey && dst == writerkey);
        ASSERT(false, "Unimplemented");
    }

    bool StreamEndpoint::ReadReady() {
        Sync::AutoReentrantLock arl(*lock);
        return PacketDecoder::Enabled();
    }

    bool StreamEndpoint::WriteReady() {
        Sync::AutoReentrantLock arl(*lock);
        return encoder.BytesReady();
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
        while (encoder.BytesReady()) {
            unsigned towrite = 0;
            const void *ptr = encoder.GetEncodedBytes(towrite);
            if (0 == towrite) break;
            unsigned numwritten = stream.Write(ptr, towrite);
            if (numwritten == 0) {
                break;
            } else {
                encoder.ReleaseEncodedBytes(numwritten);
            }
        }
    }

    void StreamEndpoint::ReceivedEnqueue(void *data, unsigned length, unsigned numchannels) {
        ASSERT(numchannels == queue->NumChannels(),
                "Endpoints disagree on number of channels.");
        // We want to send the enqueue message after we have actually enqueued the data
        ENSURE(queue->RawEnqueue(data, length, numchannels, length),
                "Enqueue throttling over the socket failed.");
        readcount += length;
        rmh->RMHEnqueue(writerkey, readerkey);
    }

    void StreamEndpoint::ReceivedDequeue(unsigned length, unsigned numchannels) {
        ASSERT(numchannels == queue->NumChannels(),
                "Endpoints disagree on number of channels.");
        writecount -= length;
        CheckBlockedEnqueues();
        wmh->WMHDequeue(readerkey, writerkey);
    }

    void StreamEndpoint::ReceivedReadBlock(unsigned requested) {
        wmh->WMHReadBlock(readerkey, writerkey);
    }

    void StreamEndpoint::ReceivedWriteBlock(unsigned requested) {
        rmh->RMHWriteBlock(writerkey, readerkey);
    }

    void StreamEndpoint::CheckBlockedEnqueues() {
        while (!EnqueueBlocked() && WriteEnqueue()) {}
    }

    bool StreamEndpoint::WriteEnqueue() {
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

