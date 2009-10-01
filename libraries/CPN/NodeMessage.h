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
 * \brief Definition of all the messages that nodes and endpoints
 * deal with.
 * \author John Bridgman
 */

#ifndef CPN_MESSAGE_NODEMESSAGE
#define CPN_MESSAGE_NODEMESSAGE
#pragma once

#include "CPNCommon.h"

namespace CPN {

    class NodeMessage {
    public:
        virtual ~NodeMessage();
        virtual void DispatchOn(NodeMsgDispatch *node) = 0;
        void SetKey(Key_t k) { key = k; }
        Key_t GetKey() const { return key; }
    private:
        Key_t key;
    };


    class NodeBroadcastMessage : public NodeMessage {
    public:
        virtual ~NodeBroadcastMessage();
        virtual shared_ptr<NodeMessage> Clone() const = 0;
    };

    class NodeShutdown : public NodeBroadcastMessage {
    public:
        static NodeShutdownPtr Create();
        shared_ptr<NodeMessage> Clone() const;
        void DispatchOn(NodeMsgDispatch *node);
    private:
        NodeShutdown() {}
    };

    class KeyMutator {
    public:
        KeyMutator(Key_t k) : key(k) {}
        NodeMessagePtr operator()(NodeMessagePtr msg) {
            msg->SetKey(key);
            return msg;
        }
    private:
        Key_t key;
    };

    class NodeEnqueue : public NodeMessage,
        public std::tr1::enable_shared_from_this<NodeEnqueue> {
    public:
        static NodeEnqueuePtr Create(unsigned amount);
        void DispatchOn(NodeMsgDispatch *node);
        unsigned Amount() const { return amount; }
    private:
        NodeEnqueue(unsigned a) : amount(a) {}
        unsigned amount;
    };

    class NodeDequeue : public NodeMessage,
        public std::tr1::enable_shared_from_this<NodeDequeue> {
    public:
        static NodeDequeuePtr Create(unsigned amount);
        void DispatchOn(NodeMsgDispatch *node);
        unsigned Amount() const { return amount; }
    private:
        NodeDequeue(unsigned a) : amount(a) {}
        unsigned amount;
    };

    class NodeEndOfReadQueue : public NodeMessage {
    public:
        static shared_ptr<NodeEndOfReadQueue> Create();
        void DispatchOn(NodeMsgDispatch *node);
    };

    class NodeEndOfWriteQueue : public NodeMessage {
    public:
        static shared_ptr<NodeEndOfWriteQueue> Create();
        void DispatchOn(NodeMsgDispatch *node);
    };

    class NodeReadBlock : public NodeMessage,
        public std::tr1::enable_shared_from_this<NodeReadBlock> {
    public:
        static NodeReadBlockPtr Create(unsigned request);
        void DispatchOn(NodeMsgDispatch *node);
        unsigned Requested() const { return requested; }
    private:
        NodeReadBlock(unsigned request) : requested(request) {}
        unsigned requested;
    };

    class NodeWriteBlock : public NodeMessage,
        public std::tr1::enable_shared_from_this<NodeWriteBlock> {
    public:
        static NodeWriteBlockPtr Create(unsigned request);
        void DispatchOn(NodeMsgDispatch *node);
        unsigned Requested() const { return requested; }
    private:
        NodeWriteBlock(unsigned request) : requested(request) {}
        unsigned requested;
    };

    class NodeSetReader : public NodeMessage,
        public std::tr1::enable_shared_from_this<NodeSetReader> {
    public:
        static NodeSetReaderPtr Create(Key_t key, shared_ptr<QueueBase> queue);
        void DispatchOn(NodeMsgDispatch *node);
        shared_ptr<QueueBase> GetQueue() const { return queue; }
    private:
        NodeSetReader(Key_t k, shared_ptr<QueueBase> q) : queue(q) {
            SetKey(k);
        }
        shared_ptr<QueueBase> queue;
    };

    class NodeSetWriter : public NodeMessage,
        public std::tr1::enable_shared_from_this<NodeSetWriter> {
    public:
        static NodeSetWriterPtr Create(Key_t key, shared_ptr<QueueBase> queue);
        void DispatchOn(NodeMsgDispatch *node);
        shared_ptr<QueueBase> GetQueue() const { return queue; }
    private:
        NodeSetWriter(Key_t k, shared_ptr<QueueBase> q) : queue(q) {
            SetKey(k);
        }
        shared_ptr<QueueBase> queue;
    };

    class NodeMsgDispatch {
    public:
        virtual ~NodeMsgDispatch();
        virtual void ProcessMessage(NodeSetReader *msg) = 0;
        virtual void ProcessMessage(NodeSetWriter *msg) = 0;
        virtual void ProcessMessage(NodeEnqueue *msg) = 0;
        virtual void ProcessMessage(NodeDequeue *msg) = 0;
        virtual void ProcessMessage(NodeReadBlock *msg) = 0;
        virtual void ProcessMessage(NodeWriteBlock *msg) = 0;
        virtual void ProcessMessage(NodeEndOfWriteQueue *msg) = 0;
        virtual void ProcessMessage(NodeEndOfReadQueue *msg) = 0;
        virtual void Shutdown() = 0;
    };
}

#endif

