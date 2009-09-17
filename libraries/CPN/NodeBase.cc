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
 * \brief Implementation for the NodeBase class.
 * \author John Bridgman
 */

#include "NodeBase.h"
#include "Kernel.h"
#include "Exceptions.h"
#include "Database.h"
#include "MessageQueue.h"
#include "QueueReader.h"
#include "QueueWriter.h"

#include "MapInvoke.h"
#include <vector>

namespace CPN {

    NodeBase::NodeBase(Kernel &ker, const NodeAttr &attr)
    : kernel(ker), name(attr.GetName()), type(attr.GetTypeName()),
    nodekey(attr.GetKey()), database(attr.GetDatabase()),
    terminated(false)
    {
        msgqueue = MsgQueue<NodeMessagePtr>::Create();
    }

    NodeBase::~NodeBase() {
    }

    void NodeBase::ProcessMessage(NodeSetReader *msg) {
        shared_ptr<QueueReader> reader = GetReader(msg->GetKey(), true);
        reader->SetQueue(msg->GetQueue());
    }

    void NodeBase::ProcessMessage(NodeSetWriter *msg) {
        shared_ptr<QueueWriter> writer = GetWriter(msg->GetKey(), true);
        writer->SetQueue(msg->GetQueue());
    }

    void NodeBase::ProcessMessage(NodeEnqueue *msg) {
        // Remove from block list
    }

    void NodeBase::ProcessMessage(NodeDequeue *msg) {
        // Remove from block list
    }

    void NodeBase::ProcessMessage(NodeReadBlock *msg) {
        // Add to block list
    }

    void NodeBase::ProcessMessage(NodeWriteBlock *msg) {
        // Add to block list
    }

    void NodeBase::ProcessMessage(NodeEndOfWriteQueue *msg) {
        shared_ptr<QueueReader> reader = GetReader(msg->GetKey(), false);
        if (reader) {
            reader->Shutdown();
        }
    }

    void NodeBase::ProcessMessage(NodeEndOfReadQueue *msg) {
        shared_ptr<QueueWriter> writer = GetWriter(msg->GetKey(), false);
        if (writer) {
            writer->Shutdown();
        }
    }

    shared_ptr<QueueReader> NodeBase::GetReader(const std::string &portname) {
        CheckTerminate();
        Key_t ekey = database->GetCreateReaderKey(nodekey, portname);
        return GetReader(ekey, true);
    }

    shared_ptr<QueueWriter> NodeBase::GetWriter(const std::string &portname) {
        CheckTerminate();
        Key_t ekey = database->GetCreateWriterKey(nodekey, portname);
        return GetWriter(ekey, true);
    }

    void* NodeBase::EntryPoint() {
        try {
            database->SignalNodeStart(nodekey);
            Process();
        } catch (CPN::ShutdownException e) {
            // Forced shutdown
        }
        // force release of all readers and writers
        // I don't like this...
        while (!readermap.empty()) {
            readermap.begin()->second->Release();
        }
        while (!writermap.empty()) {
            writermap.begin()->second->Release();
        }
        kernel.NodeTerminated(nodekey);
        return 0;
    }

    shared_ptr<QueueReader> NodeBase::GetReader(Key_t ekey, bool create) {
        shared_ptr<QueueReader> reader;
        ReaderMap::iterator entry = readermap.find(ekey);
        if (entry == readermap.end()) {
            if (create) {
                //printf("Reader %lu created\n", ekey);
                reader = shared_ptr<QueueReader>(new QueueReader(this, ekey));
                readermap.insert(std::make_pair(ekey, reader));
            }
        } else {
            reader = shared_ptr<QueueReader>(entry->second);
        }
        return reader;
    }

    shared_ptr<QueueWriter> NodeBase::GetWriter(Key_t ekey, bool create) {
        shared_ptr<QueueWriter> writer;
        WriterMap::iterator entry = writermap.find(ekey);
        if (entry == writermap.end()) {
            if (create) {
                //printf("Writer %lu created\n", ekey);
                writer = shared_ptr<QueueWriter>(new QueueWriter(this, ekey));
                writermap.insert(std::make_pair(ekey, writer));
            }
        } else {
            writer = shared_ptr<QueueWriter>(entry->second);
        }
        return writer;
    }

    void NodeBase::ProcessUntilMsgFrom(Key_t key) {
        while (true) {
            NodeMessagePtr msg = msgqueue->Get();
            msg->DispatchOn(this);
            CheckTerminate();
            if (key == msg->GetKey()) { break; }
        }
    }

    void NodeBase::ReadNeedQueue(Key_t ekey) {
        ProcessUntilMsgFrom(ekey);
    }

    void NodeBase::WriteNeedQueue(Key_t ekey) {
        ProcessUntilMsgFrom(ekey);
    }

    void NodeBase::ReadBlock(shared_ptr<QueueReader> reader, unsigned thresh) {
        reader->PutMsg(NodeReadBlock::Create(thresh));
        ProcessUntilMsgFrom(reader->GetKey());
    }

    void NodeBase::WriteBlock(shared_ptr<QueueWriter> writer, unsigned thresh) {
        writer->PutMsg(NodeWriteBlock::Create(thresh));
        ProcessUntilMsgFrom(writer->GetKey());
    }

    void NodeBase::ReleaseReader(Key_t ekey) {
        if (readermap.find(ekey) != readermap.end()) {
            database->DestroyReaderKey(ekey);
            readermap.erase(ekey);
        }
    }

    void NodeBase::ReleaseWriter(Key_t ekey) {
        if (writermap.find(ekey) != writermap.end()) {
            database->DestroyWriterKey(ekey);
            writermap.erase(ekey);
        }
    }

    void NodeBase::Shutdown() {
        Sync::AutoReentrantLock arl(lock);
        if (!terminated) {
            terminated = true;
            msgqueue->Put(NodeShutdown::Create());
        }
    }

    void NodeBase::CheckTerminate() {
        Sync::AutoReentrantLock arl(lock);
        if (terminated) {
            throw ShutdownException();
        }
    }
}

