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
 * \brief Implementation for the node messages
 * \author John Bridgman
 */

#include "NodeMessage.h"

namespace CPN {

    NodeMessage::~NodeMessage() {}

    NodeBroadcastMessage::~NodeBroadcastMessage() {}


    NodeShutdownPtr NodeShutdown::Create() {
        return NodeShutdownPtr(new NodeShutdown());
    }

    shared_ptr<NodeMessage> NodeShutdown::Clone() const {
        return NodeShutdownPtr(new NodeShutdown(*this));
    }

    void NodeShutdown::DispatchOn(NodeMsgDispatch *node) {
        node->Shutdown();
    }

    NodeEnqueuePtr NodeEnqueue::Create(unsigned amount) {
        return NodeEnqueuePtr(new NodeEnqueue(amount));
    }

    void NodeEnqueue::DispatchOn(NodeMsgDispatch *node) {
        node->ProcessMessage(this);
    }


    NodeDequeuePtr NodeDequeue::Create(unsigned amount) {
        return NodeDequeuePtr(new NodeDequeue(amount));
    }

    void NodeDequeue::DispatchOn(NodeMsgDispatch *node) {
        node->ProcessMessage(this);
    }

    shared_ptr<NodeEndOfReadQueue> NodeEndOfReadQueue::Create() {
        return shared_ptr<NodeEndOfReadQueue>(new NodeEndOfReadQueue);
    }

    void NodeEndOfReadQueue::DispatchOn(NodeMsgDispatch *node) {
        node->ProcessMessage(this);
    }

    shared_ptr<NodeEndOfWriteQueue> NodeEndOfWriteQueue::Create() {
        return shared_ptr<NodeEndOfWriteQueue>(new NodeEndOfWriteQueue);
    }

    void NodeEndOfWriteQueue::DispatchOn(NodeMsgDispatch *node) {
        node->ProcessMessage(this);
    }

    NodeReadBlockPtr NodeReadBlock::Create(unsigned request) {
        return NodeReadBlockPtr(new NodeReadBlock(request));
    }

    void NodeReadBlock::DispatchOn(NodeMsgDispatch *node) {
        node->ProcessMessage(this);
    }


    NodeWriteBlockPtr NodeWriteBlock::Create(unsigned request) {
        return NodeWriteBlockPtr(new NodeWriteBlock(request));
    }

    void NodeWriteBlock::DispatchOn(NodeMsgDispatch *node) {
        node->ProcessMessage(this);
    }


    NodeSetReaderPtr NodeSetReader::Create(Key_t key, shared_ptr<QueueBase> queue) {
        return NodeSetReaderPtr(new NodeSetReader(key, queue));
    }

    void NodeSetReader::DispatchOn(NodeMsgDispatch *node) {
        node->ProcessMessage(this);
    }


    NodeSetWriterPtr NodeSetWriter::Create(Key_t key, shared_ptr<QueueBase> queue) {
        return NodeSetWriterPtr(new NodeSetWriter(key, queue));
    }

    void NodeSetWriter::DispatchOn(NodeMsgDispatch *node) {
        node->ProcessMessage(this);
    }

    NodeMsgDispatch::~NodeMsgDispatch() {}
}

