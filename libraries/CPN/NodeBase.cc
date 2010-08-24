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
#include "D4RDeadlockException.h"
#include "ErrnoException.h"
#include "PthreadFunctional.h"

#include <vector>

namespace CPN {

    NodeBase::NodeBase(Kernel &ker, const NodeAttr &attr)
    :   kernel(ker),
        name(attr.GetName()),
        type(attr.GetTypeName()),
        nodekey(attr.GetKey()),
        d4rnode(new D4R::Node(attr.GetKey())),
        database(attr.GetDatabase())
    {
        thread.reset(CreatePthreadFunctional(this, &NodeBase::EntryPoint));
    }

    NodeBase::~NodeBase() {
    }

    void NodeBase::Start() {
        thread->Start();
    }

    void NodeBase::Join() {
        thread->Join();
    }

    void NodeBase::CreateReader(shared_ptr<QueueBase> q) {
        Sync::AutoReentrantLock arl(lock);
        Key_t readerkey = q->GetReaderKey();
        d4rnode->AddReader(q);
        q->SetReaderNode(d4rnode);
        q->SignalReaderTagChanged();
        ASSERT(readermap.find(readerkey) == readermap.end(), "The reader already exists");
        shared_ptr<QueueReader> reader;
        reader = shared_ptr<QueueReader>(new QueueReader(this, q));
        readermap.insert(std::make_pair(readerkey, reader));
        cond.Signal();
    }

    void NodeBase::CreateWriter(shared_ptr<QueueBase> q) {
        Sync::AutoReentrantLock arl(lock);
        Key_t writerkey = q->GetWriterKey();
        d4rnode->AddWriter(q);
        q->SetWriterNode(d4rnode);
        q->SignalWriterTagChanged();
        ASSERT(writermap.find(writerkey) == writermap.end(), "The writer already exists.");
        shared_ptr<QueueWriter> writer;
        writer = shared_ptr<QueueWriter>(new QueueWriter(this, q));
        writermap.insert(std::make_pair(writerkey, writer));
        cond.Signal();
    }

    shared_ptr<QueueReader> NodeBase::GetReader(const std::string &portname) {
        database->CheckTerminated();
        Key_t ekey = database->GetCreateReaderKey(nodekey, portname);
        return GetReader(ekey);
    }

    shared_ptr<QueueWriter> NodeBase::GetWriter(const std::string &portname) {
        database->CheckTerminated();
        Key_t ekey = database->GetCreateWriterKey(nodekey, portname);
        return GetWriter(ekey);
    }

    void* NodeBase::EntryPoint() {
        Sync::AutoReentrantLock arl(lock, false);
        try {
            database->SignalNodeStart(nodekey);
            Process();
        } catch (const CPN::ShutdownException &e) {
            // Forced shutdown
        } catch (const CPN::BrokenQueueException &e) {
            if (!database->SwallowBrokenQueueExceptions()) {
                throw;
            }
        } catch (const D4R::DeadlockException &e) {
            // A true deadlock was detected, die
            Logger logger(database.get(), Logger::ERROR);
            logger.Name(name.c_str());
            logger.Info("DEADLOCK detected at %s\n", name.c_str());
        }
        database->SignalNodeEnd(nodekey);
        kernel.NodeTerminated(nodekey);
        // force release of all readers and writers
        arl.Lock();
        ReaderMap readers;
        readers.swap(readermap);
        WriterMap writers;
        writers.swap(writermap);
        arl.Unlock();
        readers.clear();
        writers.clear();
        return 0;
    }

    shared_ptr<QueueReader> NodeBase::GetReader(Key_t ekey) {
        Sync::AutoReentrantLock arl(lock);
        shared_ptr<QueueReader> reader;
        while (!reader) {
            ReaderMap::iterator entry = readermap.find(ekey);
            if (entry == readermap.end()) {
                database->CheckTerminated();
                cond.Wait(lock);
            } else {
                reader = shared_ptr<QueueReader>(entry->second);
            }
        }
        return reader;
    }

    shared_ptr<QueueWriter> NodeBase::GetWriter(Key_t ekey) {
        Sync::AutoReentrantLock arl(lock);
        shared_ptr<QueueWriter> writer;
        while (!writer) {
            WriterMap::iterator entry = writermap.find(ekey);
            if (entry == writermap.end()) {
                database->CheckTerminated();
                cond.Wait(lock);
            } else {
                writer = shared_ptr<QueueWriter>(entry->second);
            }
        }
        return writer;
    }

    void NodeBase::ReleaseReader(Key_t ekey) {
        shared_ptr<QueueReader> reader;
        Sync::AutoReentrantLock arl(lock);
        ReaderMap::iterator entry = readermap.find(ekey);
        if (entry != readermap.end()) {
            reader = entry->second;
            readermap.erase(entry);
        }
        arl.Unlock();
        reader.reset();
    }

    void NodeBase::ReleaseWriter(Key_t ekey) {
        shared_ptr<QueueWriter> writer;
        Sync::AutoReentrantLock arl(lock);
        WriterMap::iterator entry = writermap.find(ekey);
        if (entry != writermap.end()) {
            writer = entry->second;
            writermap.erase(entry);
        }
        arl.Unlock();
        writer.reset();
    }

    void NodeBase::NotifyTerminate() {
        Sync::AutoReentrantLock arl(lock);
        cond.Signal();
        WriterMap::iterator witr = writermap.begin();
        while (witr != writermap.end()) { (witr++)->second->NotifyTerminate(); }
        ReaderMap::iterator ritr = readermap.begin();
        while (ritr != readermap.end()) { (ritr++)->second->NotifyTerminate(); }
    }

    void NodeBase::LogState() {
        Logger logger(database.get(), Logger::ERROR);
        logger.Name(name.c_str());
        logger.Error("Logging (key: %llu), %u readers, %u writers, %s",
                nodekey, readermap.size(), writermap.size(), thread->Running() ? "Running" : "done");
        logger.Error("Thread id: %llu", (unsigned long long)((pthread_t)(*thread.get())));
        D4R::Tag tag = d4rnode->GetPrivateTag();
        logger.Error("Private key: (%llu, %llu, %llu, %llu)", tag.Count(), tag.Key(), tag.QueueSize(), tag.QueueKey());
        tag = d4rnode->GetPublicTag();
        logger.Error("Public key: (%llu, %llu, %llu, %llu)", tag.Count(), tag.Key(), tag.QueueSize(), tag.QueueKey());
        ReaderMap::iterator r = readermap.begin();
        while (r != readermap.end()) {
            r->second->GetQueue()->LogState();
            ++r;
        }
        WriterMap::iterator w = writermap.begin();
        while (w != writermap.end()) {
            w->second->GetQueue()->LogState();
            ++w;
        }
    }
}

