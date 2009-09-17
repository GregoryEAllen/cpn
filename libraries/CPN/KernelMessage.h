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
 * \brief Definition of all the messages that the kernel
 * deals with.
 * \author John Bridgman
 */

#ifndef CPN_KERNELMESSAGE_H
#define CPN_KERNELMESSAGE_H
#pragma once

#include "common.h"
#include "QueueAttr.h"
#include "NodeAttr.h"
#include "Assert.h"
#include "AsyncStream.h"

namespace CPN {

    class KMsgDispatchable;

    class KernelMessage {
    public:
        virtual ~KernelMessage();
        virtual void DispatchOn(KMsgDispatchable *k) = 0;
        void SetDestinationKey(Key_t k) { destkey = k; }
        Key_t GetDestinationKey() const { return destkey; }
    protected:
        KernelMessage() : destkey(0) {}
        KernelMessage(Key_t k) : destkey(k) {}
        Key_t destkey;
    };

    typedef shared_ptr<KernelMessage> KernelMessagePtr;

    class KMsgCreateWriter : public KernelMessage {
    public:
        static shared_ptr<KMsgCreateWriter> Create(const SimpleQueueAttr &attr, Key_t hostkey);
        virtual void DispatchOn(KMsgDispatchable *k);
        SimpleQueueAttr &GetAttr() { return attr; }
    private:
        KMsgCreateWriter(const SimpleQueueAttr &a, Key_t hostkey);
        SimpleQueueAttr attr;
    };

    class KMsgCreateReader : public KernelMessage {
    public:
        static shared_ptr<KMsgCreateReader> Create(const SimpleQueueAttr &attr, Key_t hostkey);
        virtual void DispatchOn(KMsgDispatchable *k);
        SimpleQueueAttr &GetAttr() { return attr; }
    private:
        KMsgCreateReader(const SimpleQueueAttr &a, Key_t hostkey);
        SimpleQueueAttr attr;
    };

    class KMsgCreateQueue : public KernelMessage {
    public:
        static shared_ptr<KMsgCreateQueue> Create(const SimpleQueueAttr &attr, Key_t hostkey);
        virtual void DispatchOn(KMsgDispatchable *k);
        SimpleQueueAttr &GetAttr() { return attr; }
    private:
        KMsgCreateQueue(const SimpleQueueAttr &a, Key_t hostkey);
        SimpleQueueAttr attr;
    };

    class KMsgCreateNode : public KernelMessage {
    public:
        static shared_ptr<KMsgCreateNode> Create(const NodeAttr &attr);
        virtual void DispatchOn(KMsgDispatchable *k);
        NodeAttr &GetAttr() { return attr; }
    private:
        KMsgCreateNode(const NodeAttr &a);
        NodeAttr attr;
    };

    class KMsgStreamDead : public KernelMessage {
    public:
        static shared_ptr<KMsgStreamDead> Create(Key_t streamkey);
        virtual void DispatchOn(KMsgDispatchable *k);
        Key_t GetStreamKey() const { return streamkey; }
    private:
        KMsgStreamDead(Key_t skey);
        Key_t streamkey;
    };

    class KMsgSetReaderDescriptor : public KernelMessage {
    public:
        static shared_ptr<KMsgSetReaderDescriptor> Create(Key_t readerkey, Async::DescriptorPtr desc);
        virtual void DispatchOn(KMsgDispatchable *k);
        Key_t GetReaderKey() const { return readerkey; }
        Async::DescriptorPtr GetDescriptor() const { return descriptor; }
    private:
        KMsgSetReaderDescriptor(Key_t rkey, Async::DescriptorPtr desc);
        Key_t readerkey;
        Async::DescriptorPtr descriptor;
    };

    class KMsgSetWriterDescriptor : public KernelMessage {
    public:
        static shared_ptr<KMsgSetWriterDescriptor> Create(Key_t writerkey, Async::DescriptorPtr desc);
        virtual void DispatchOn(KMsgDispatchable *k);
        Key_t GetWriterKey() const { return writerkey; }
        Async::DescriptorPtr GetDescriptor() const { return descriptor; }
    private:
        KMsgSetWriterDescriptor(Key_t wkey, Async::DescriptorPtr desc);
        Key_t writerkey;
        Async::DescriptorPtr descriptor;
    };

    class KMsgCreateKernelStream : public KernelMessage {
    public:
        static shared_ptr<KMsgCreateKernelStream> Create(Key_t kernelkey, Async::DescriptorPtr desc);
        virtual void DispatchOn(KMsgDispatchable *k);
        Key_t GetKernelKey() const { return kernelkey; }
        Async::DescriptorPtr GetDescriptor() const { return descriptor; }
    private:
        KMsgCreateKernelStream(Key_t kkey, Async::DescriptorPtr desc);
        Key_t kernelkey;
        Async::DescriptorPtr descriptor;
    };

    class KMsgDispatchable {
    public:
        virtual ~KMsgDispatchable();
        virtual void ProcessMessage(KMsgCreateWriter *msg) = 0;
        virtual void ProcessMessage(KMsgCreateReader *msg) = 0;
        virtual void ProcessMessage(KMsgCreateQueue *msg) = 0;
        virtual void ProcessMessage(KMsgCreateNode *msg) = 0;

        virtual void ProcessMessage(KMsgStreamDead *msg) { ASSERT(false, "Unimplemented"); }
        virtual void ProcessMessage(KMsgSetReaderDescriptor *msg) { ASSERT(false, "Unimplemented"); }
        virtual void ProcessMessage(KMsgSetWriterDescriptor *msg) { ASSERT(false, "Unimplemented"); }
        virtual void ProcessMessage(KMsgCreateKernelStream *msg) { ASSERT(false, "Unimplemented"); }
    };
}

#endif

