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
#include "QueueReader.h"
#include "QueueWriter.h"

#include "MapInvoke.h"
#include <vector>

namespace CPN {

    NodeBase::NodeBase(Kernel &ker, const NodeAttr &attr)
    : kernel(ker), name(attr.GetName()), type(attr.GetTypeName()),
    nodekey(attr.GetKey()), database(attr.GetDatabase()),
    terminated(false), blockkey(0)
    {
    }

    NodeBase::~NodeBase() {
    }

    void NodeBase::CreateReader(Key_t readerkey, Key_t writerkey, shared_ptr<QueueBase> q) {
        Sync::AutoReentrantLock arl(lock);
        ASSERT(readermap.find(readerkey) == readermap.end(), "The reader already exists");
        shared_ptr<QueueReader> reader;
        reader = shared_ptr<QueueReader>(new QueueReader(this, this, readerkey, writerkey, q));
        readermap.insert(std::make_pair(readerkey, reader));
        Unblock(readerkey);
    }

    void NodeBase::CreateWriter(Key_t readerkey, Key_t writerkey, shared_ptr<QueueBase> q) {
        Sync::AutoReentrantLock arl(lock);
        ASSERT(writermap.find(writerkey) == writermap.end(), "The writer already exists.");
        shared_ptr<QueueWriter> writer;
        writer = shared_ptr<QueueWriter>(new QueueWriter(this, this, writerkey, readerkey, q));
        writermap.insert(std::make_pair(writerkey, writer));
        Unblock(writerkey);
    }

    void NodeBase::RMHEnqueue(Key_t writerkey, Key_t readerkey) {
        Sync::AutoReentrantLock arl(lock);
        Unblock(writerkey);
    }

    void NodeBase::RMHEndOfWriteQueue(Key_t writerkey, Key_t readerkey) {
        Sync::AutoReentrantLock arl(lock);
        Unblock(writerkey);
    }


    void NodeBase::RMHWriteBlock(Key_t writerkey, Key_t readerkey, unsigned requested) {
        Sync::AutoReentrantLock arl(lock);
        Unblock(writerkey);
    }

    void NodeBase::RMHTagChange(Key_t writerkey, Key_t readerkey) {
        Sync::AutoReentrantLock arl(lock);
        Unblock(writerkey);
    }

    void NodeBase::WMHDequeue(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arl(lock);
        Unblock(readerkey);
    }
    void NodeBase::WMHEndOfReadQueue(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arl(lock);
        Unblock(readerkey);
    }

    void NodeBase::WMHReadBlock(Key_t readerkey, Key_t writerkey, unsigned requested) {
        Sync::AutoReentrantLock arl(lock);
        Unblock(readerkey);
    }

    void NodeBase::WMHTagChange(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arl(lock);
        Unblock(readerkey);
    }

    shared_ptr<QueueReader> NodeBase::GetReader(const std::string &portname) {
        Sync::AutoReentrantLock arl(lock);
        CheckTerminate();
        Key_t ekey = database->GetCreateReaderKey(nodekey, portname);
        return GetReader(ekey, true);
    }

    shared_ptr<QueueWriter> NodeBase::GetWriter(const std::string &portname) {
        Sync::AutoReentrantLock arl(lock);
        CheckTerminate();
        Key_t ekey = database->GetCreateWriterKey(nodekey, portname);
        return GetWriter(ekey, true);
    }

    void* NodeBase::EntryPoint() {
        Sync::AutoReentrantLock arl(lock, false);
        try {
            database->SignalNodeStart(nodekey);
            Process();
        } catch (CPN::ShutdownException e) {
            // Forced shutdown
        }
        arl.Lock();
        // force release of all readers and writers
        while (!readermap.empty()) {
            shared_ptr<QueueReader> reader = readermap.begin()->second;
            arl.Unlock();
            reader->Release();
            reader.reset();
            arl.Lock();
        }
        while (!writermap.empty()) {
            shared_ptr<QueueWriter> writer = writermap.begin()->second;
            arl.Unlock();
            writer->Release();
            writer.reset();
            arl.Lock();
        }
        arl.Unlock();
        database->SignalNodeEnd(nodekey);
        kernel.NodeTerminated(nodekey);
        return 0;
    }

    shared_ptr<QueueReader> NodeBase::GetReader(Key_t ekey, bool block) {
        shared_ptr<QueueReader> reader;
        while (!reader) {
            ReaderMap::iterator entry = readermap.find(ekey);
            if (entry == readermap.end()) {
                if (block) {
                    Block(ekey);
                } else {
                    break;
                }
            } else {
                reader = shared_ptr<QueueReader>(entry->second);
            }
        }
        return reader;
    }

    shared_ptr<QueueWriter> NodeBase::GetWriter(Key_t ekey, bool block) {
        shared_ptr<QueueWriter> writer;
        while (!writer) {
            WriterMap::iterator entry = writermap.find(ekey);
            if (entry == writermap.end()) {
                if (block) {
                    Block(ekey);
                } else {
                    break;
                }
            } else {
                writer = shared_ptr<QueueWriter>(entry->second);
            }
        }
        return writer;
    }

    void NodeBase::Block(Key_t ekey) {
        Sync::AutoReentrantLock arl(lock);
        BlockSet::iterator entry = blockset.find(ekey);
        ASSERT(blockkey == 0);
        blockkey = ekey;
        while (blockkey == ekey) {
            CheckTerminate();
            if (entry != blockset.end()) {
                blockset.erase(entry);
                break;
            }
            cond.Wait(lock);
        }
        blockkey = 0;
    }

    void NodeBase::Unblock(Key_t ekey) {
        Sync::AutoReentrantLock arl(lock);
        if (blockkey == ekey) {
            blockkey = 0;
            cond.Signal();
        } else {
            blockset.insert(ekey);
        }
    }

    void NodeBase::ReadBlock(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arl(lock);
        Block(writerkey);
    }

    void NodeBase::WriteBlock(Key_t writerkey, Key_t readerkey) {
        Sync::AutoReentrantLock arl(lock);
        Block(readerkey);
    }

    void NodeBase::ReleaseReader(Key_t ekey) {
        Sync::AutoReentrantLock arl(lock);
        if (readermap.find(ekey) != readermap.end()) {
            database->DestroyReaderKey(ekey);
            readermap.erase(ekey);
        }
    }

    void NodeBase::ReleaseWriter(Key_t ekey) {
        Sync::AutoReentrantLock arl(lock);
        if (writermap.find(ekey) != writermap.end()) {
            database->DestroyWriterKey(ekey);
            writermap.erase(ekey);
        }
    }

    void NodeBase::Shutdown() {
        Sync::AutoReentrantLock arl(lock);
        if (!terminated) {
            terminated = true;
            cond.Signal();
        }
    }

    void NodeBase::CheckTerminate() {
        Sync::AutoReentrantLock arl(lock);
        if (terminated) {
            throw ShutdownException();
        }
    }
}

