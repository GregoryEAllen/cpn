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
 * \brief kernel message implementations
 * \author John Bridgman
 */

#include "KernelMessage.h"

namespace CPN {
    KernelMessage::~KernelMessage() {}

    shared_ptr<KMsgCreateWriter> KMsgCreateWriter::Create(const SimpleQueueAttr &attr,
            Key_t hostkey) {
        return shared_ptr<KMsgCreateWriter>( new KMsgCreateWriter(attr, hostkey));
    }

    KMsgCreateWriter::KMsgCreateWriter(const SimpleQueueAttr &a, Key_t hostkey)
        : KernelMessage(hostkey), attr(a) {
    }

    void KMsgCreateWriter::DispatchOn(KMsgDispatchable *k) {
        k->ProcessMessage(this);
    }

    shared_ptr<KMsgCreateReader> KMsgCreateReader::Create(const SimpleQueueAttr &attr,
            Key_t hostkey) {
        return shared_ptr<KMsgCreateReader>( new KMsgCreateReader(attr, hostkey));
    }

    KMsgCreateReader::KMsgCreateReader(const SimpleQueueAttr &a, Key_t hostkey)
        : KernelMessage(hostkey), attr(a) {
    }

    void KMsgCreateReader::DispatchOn(KMsgDispatchable *k) {
        k->ProcessMessage(this);
    }

    shared_ptr<KMsgCreateQueue> KMsgCreateQueue::Create(const SimpleQueueAttr &attr,
            Key_t hostkey) {
        return shared_ptr<KMsgCreateQueue>( new KMsgCreateQueue(attr, hostkey));
    }

    KMsgCreateQueue::KMsgCreateQueue(const SimpleQueueAttr &a, Key_t hostkey)
        : KernelMessage(hostkey), attr(a) {
    }

    void KMsgCreateQueue::DispatchOn(KMsgDispatchable *k) {
        k->ProcessMessage(this);
    }

    shared_ptr<KMsgCreateNode> KMsgCreateNode::Create(const NodeAttr &attr) {
        return shared_ptr<KMsgCreateNode>( new KMsgCreateNode(attr));
    }

    KMsgCreateNode::KMsgCreateNode(const NodeAttr &a)
        : KernelMessage(a.GetHostKey()), attr(a) {
    }

    void KMsgCreateNode::DispatchOn(KMsgDispatchable *k) {
        k->ProcessMessage(this);
    }

    shared_ptr<KMsgStreamDead> KMsgStreamDead::Create(Key_t streamkey) {
        return shared_ptr<KMsgStreamDead>(new KMsgStreamDead(streamkey));
    }

    KMsgStreamDead::KMsgStreamDead(Key_t skey) : streamkey(skey) {}

    void KMsgStreamDead::DispatchOn(KMsgDispatchable *k) {
        k->ProcessMessage(this);
    }

    shared_ptr<KMsgSetWriterDescriptor> KMsgSetWriterDescriptor::Create(
            Key_t writerkey, Async::DescriptorPtr desc) {
        return shared_ptr<KMsgSetWriterDescriptor>(new KMsgSetWriterDescriptor(writerkey, desc));
    }

    KMsgSetWriterDescriptor::KMsgSetWriterDescriptor(Key_t wkey, Async::DescriptorPtr desc)
        : writerkey(wkey), descriptor(desc) {}

    void KMsgSetWriterDescriptor::DispatchOn(KMsgDispatchable *k) {
        k->ProcessMessage(this);
    }

    shared_ptr<KMsgSetReaderDescriptor> KMsgSetReaderDescriptor::Create(
            Key_t readerkey, Async::DescriptorPtr desc) {
        return shared_ptr<KMsgSetReaderDescriptor>(new KMsgSetReaderDescriptor(readerkey, desc));
    }

    KMsgSetReaderDescriptor::KMsgSetReaderDescriptor(
            Key_t rkey, Async::DescriptorPtr desc)
        : readerkey(rkey), descriptor(desc) {}

    void KMsgSetReaderDescriptor::DispatchOn(KMsgDispatchable *k) {
        k->ProcessMessage(this);
    }

    shared_ptr<KMsgCreateKernelStream> KMsgCreateKernelStream::Create(
            Key_t kernelkey, Async::DescriptorPtr desc) {
        return shared_ptr<KMsgCreateKernelStream>(new KMsgCreateKernelStream(kernelkey, desc));
    }

    KMsgCreateKernelStream::KMsgCreateKernelStream(
            Key_t kkey, Async::DescriptorPtr desc)
        : kernelkey(kkey), descriptor(desc) {}

    void KMsgCreateKernelStream::DispatchOn(KMsgDispatchable *k) {
        k->ProcessMessage(this);
    }

    KMsgDispatchable::~KMsgDispatchable() {}
}

