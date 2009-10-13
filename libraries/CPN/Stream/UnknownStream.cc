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

#include "UnknownStream.h"

namespace CPN {
    UnknownStream::UnknownStream(Async::SockPtr desc,
            KernelMessageHandler *kernMsgHan)
        : descriptor(desc), readerkey(0), writerkey(0), mode(UNKNOWN), dead(false)
    {
        PacketDecoder::Enable(true);
        SetupDescriptor();
    }

    UnknownStream::UnknownStream(Async::SockPtr desc,
            KernelMessageHandler *kernMsgHan,
            Key_t rkey, Key_t wkey, Mode_t m)
        : descriptor(desc), readerkey(rkey), writerkey(wkey), mode(m), dead(false)
    {
        if (mode == READ) {
            encoder.SendReaderID(readerkey, writerkey);
        } else if (mode == WRITE) {
            encoder.SendWriterID(writerkey, readerkey);
        } else {
            ASSERT(false, "Incorrect UnknownStream state");
        }
        PacketDecoder::Enable(false);
        SetupDescriptor();
    }

    void UnknownStream::ReceivedReaderID(uint64_t rkey, uint64_t wkey) {
        kmh->SetWriterDescriptor(wkey, rkey, descriptor);
        PacketDecoder::Enable(false);
        descriptor.reset();
        dead = true;
    }

    void UnknownStream::ReceivedWriterID(uint64_t wkey, uint64_t rkey) {
        kmh->SetReaderDescriptor(rkey, wkey, descriptor);
        PacketDecoder::Enable(false);
        descriptor.reset();
        dead = true;
    }

    void UnknownStream::ReceivedKernelID(uint64_t srckernelkey, uint64_t dstkernelkey) {
    }

    bool UnknownStream::ReadReady() {
        return mode == UNKNOWN && PacketDecoder::Enabled();
    }

    bool UnknownStream::WriteReady() {
        return (mode == READ || mode == WRITE) && encoder.BytesReady();
    }

    void UnknownStream::ReadSome() {
        if (dead) { return; }
        if (!descriptor) { return; }
        Async::Stream stream(descriptor);
        unsigned numtoread = 0;
        unsigned numread = 0;
        bool loop = true;
        while (loop && !dead) {
            void *ptr = PacketDecoder::GetDecoderBytes(numtoread);
            numread = stream.Read(ptr, numtoread);
            if (0 == numread) {
                if  (!stream) {
                    // The other end closed the connection!!
                    stream.Close();
                    descriptor.reset();
                    dead = true;
                }
                loop = false;
            } else {
                PacketDecoder::ReleaseDecoderBytes(numread);
            }
        }
    }

    void UnknownStream::WriteSome() {
        if (dead) { return; }
        if (!descriptor) { return; }
        Async::Stream stream(descriptor);
        while (!dead) {
            unsigned towrite = 0;
            const void *ptr = encoder.GetEncodedBytes(towrite);
            if (0 == towrite) {
                if (mode == WRITE) {
                    kmh->SetWriterDescriptor(writerkey, readerkey, descriptor);
                } else if (mode == READ) {
                    kmh->SetReaderDescriptor(readerkey, writerkey, descriptor);
                } else {
                    ASSERT(false, "Bad UnknownStream state");
                }
                dead = true;
                descriptor.reset();
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

    void UnknownStream::Error(int err) {
        descriptor.reset();
        dead = true;
    }

    void UnknownStream::SetupDescriptor() {
        descriptor->ConnectOnRead(sigc::mem_fun(this, &UnknownStream::ReadSome));
        descriptor->ConnectReadable(sigc::mem_fun(this, &UnknownStream::ReadReady));
        descriptor->ConnectOnWrite(sigc::mem_fun(this, &UnknownStream::WriteSome));
        descriptor->ConnectWriteable(sigc::mem_fun(this, &UnknownStream::WriteReady));
        descriptor->ConnectOnError(sigc::mem_fun(this, &UnknownStream::Error));
    }

}

