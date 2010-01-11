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
/**
 * \author John Bridgman
 */

#include "Message.h"

namespace CPN {

    ReaderMessageHandler::ReaderMessageHandler() : subhandler(0) {}
    ReaderMessageHandler::ReaderMessageHandler(ReaderMessageHandler *shan) : subhandler(shan) {}
    ReaderMessageHandler::~ReaderMessageHandler() { subhandler = 0; }
    void ReaderMessageHandler::RMHEnqueue(Key_t src, Key_t dst) {
        if (CheckRMH()) subhandler->RMHEnqueue(src, dst);
    }
    void ReaderMessageHandler::RMHEndOfWriteQueue(Key_t src, Key_t dst) {
        if (CheckRMH()) subhandler->RMHEndOfWriteQueue(src, dst);
    }
    void ReaderMessageHandler::RMHWriteBlock(Key_t src, Key_t dst, unsigned requested) {
        if (CheckRMH()) subhandler->RMHWriteBlock(src, dst, requested);
    }
    void ReaderMessageHandler::RMHTagChange(Key_t src, Key_t dst) {
        if (CheckRMH()) subhandler->RMHTagChange(src, dst);
    }


    WriterMessageHandler::WriterMessageHandler() : subhandler(0) {}
    WriterMessageHandler::WriterMessageHandler(WriterMessageHandler *shan) : subhandler(shan) {}
    WriterMessageHandler::~WriterMessageHandler() { subhandler = 0; }
    void WriterMessageHandler::WMHDequeue(Key_t src, Key_t dst) {
        if (CheckWMH()) subhandler->WMHDequeue(src, dst);
    }
    void WriterMessageHandler::WMHEndOfReadQueue(Key_t src, Key_t dst) {
        if (CheckWMH()) subhandler->WMHEndOfReadQueue(src, dst);
    }
    void WriterMessageHandler::WMHReadBlock(Key_t src, Key_t dst, unsigned requested) {
        if (CheckWMH()) subhandler->WMHReadBlock(src, dst, requested);
    }
    void WriterMessageHandler::WMHTagChange(Key_t src, Key_t dst) {
        if (CheckWMH()) subhandler->WMHTagChange(src, dst);
    }

    NodeMessageHandler::~NodeMessageHandler() {}

    KernelMessageHandler::~KernelMessageHandler() {}

    void KernelMessageHandler::CreateWriter(Key_t dst, const SimpleQueueAttr &attr) {
        ASSERT(false, "Unexpected message");
    }
    void KernelMessageHandler::CreateReader(Key_t dst, const SimpleQueueAttr &attr) {
        ASSERT(false, "Unexpected message");
    }
    void KernelMessageHandler::CreateQueue(Key_t dst, const SimpleQueueAttr &attr) {
        ASSERT(false, "Unexpected message");
    }
    void KernelMessageHandler::CreateNode(Key_t dst, const NodeAttr &attr) {
        ASSERT(false, "Unexpected message");
    }
    shared_ptr<Future<int> > KernelMessageHandler::GetReaderDescriptor(Key_t readerkey, Key_t writerkey) {
        ASSERT(false, "Unexpected message");
        return shared_ptr<Future<int> >();
    }
    shared_ptr<Future<int> > KernelMessageHandler::GetWriterDescriptor(Key_t readerkey, Key_t writerkey) {
        ASSERT(false, "Unexpected message");
        return shared_ptr<Future<int> >();
    }
    void KernelMessageHandler::SendWakeup() {
        ASSERT(false, "Unexpected message");
    }

    void KernelMessageHandler::NotifyTerminate() {
        ASSERT(false, "Unexpected message");
    }
}

