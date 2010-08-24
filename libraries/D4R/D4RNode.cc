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
#include "D4RNode.h"
#include "D4RQueue.h"
#include "AutoLock.h"
#include <algorithm>

#if _DEBUG
#include <stdio.h>
#define DEBUG(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

namespace D4R {

    Node::Node(uint64_t key)
        : publicTag(key),
        privateTag(key)
    {
    }

    Node::~Node() {}

    Tag Node::GetPublicTag() const {
        AutoLock<PthreadMutex> al(taglock);
        return publicTag;
    }

    void Node::SetPublicTag(const Tag &t) {
        AutoLock<PthreadMutex> al(taglock);
        publicTag = t;
    }

    Tag Node::GetPrivateTag() const {
        AutoLock<PthreadMutex> al(taglock);
        return privateTag;
    }

    void Node::SetPrivateTag(const Tag &t) {
        AutoLock<PthreadMutex> al(taglock);
        privateTag = t;
    }

    void Node::Block(const Tag &t, unsigned qsize) {
        AutoLock<PthreadMutex> al(taglock);
        privateTag.QueueSize(qsize);
        privateTag.Count(std::max(privateTag.Count(), t.Count()) + 1);
        DEBUG("Node %llu:%llu block %d\n", privateTag.Count(), privateTag.Key(), (int)privateTag.QueueSize());
        publicTag = privateTag;
        al.Unlock();
        SignalTagChanged();
    }

    bool Node::Transmit(const Tag &t) {
        AutoLock<PthreadMutex> al(taglock);
        if (publicTag < t) {

            DEBUG("Transfer: publicTag < t\n\tPrivate: (%llu, %llu, %d, %llu)\n\tPublic: (%llu, %llu, %d, %llu)\n\t     t: (%llu, %llu, %d, %llu)\n",
                    privateTag.Count(), privateTag.Key(), (int)privateTag.QueueSize(), privateTag.QueueKey(),
                    publicTag.Count(), publicTag.Key(), (int)publicTag.QueueSize(), publicTag.QueueKey(),
                    t.Count(), t.Key(), (int)t.QueueSize(), t.QueueKey());

            uint128_t priority = std::min(privateTag.Priority(), t.Priority());
            publicTag = t;
            publicTag.Priority(priority);
            al.Unlock();
            SignalTagChanged();
        } else if (publicTag == t) {

            DEBUG("Transfer: publicTag == t\n\tPrivate: (%llu, %llu, %d, %llu)\n\tPublic: (%llu, %llu, %d, %llu)\n\t     t: (%llu, %llu, %d, %llu)\n",
                    privateTag.Count(), privateTag.Key(), (int)privateTag.QueueSize(), privateTag.QueueKey(),
                    publicTag.Count(), publicTag.Key(), (int)publicTag.QueueSize(), publicTag.QueueKey(),
                    t.Count(), t.Key(), (int)t.QueueSize(), t.QueueKey());

            return publicTag.Priority() == privateTag.Priority();
        } else {
            DEBUG("Transfer: publicTag > t NOP\n\tPrivate: (%llu, %llu, %d, %llu)\n\tPublic: (%llu, %llu, %d, %llu)\n\t     t: (%llu, %llu, %d, %llu)\n",
                    privateTag.Count(), privateTag.Key(), (int)privateTag.QueueSize(), privateTag.QueueKey(),
                    publicTag.Count(), publicTag.Key(), (int)publicTag.QueueSize(), publicTag.QueueKey(),
                    t.Count(), t.Key(), (int)t.QueueSize(), t.QueueKey());
        }
        return false;
    }

    void Node::AddReader(weak_ptr<QueueBase> q) {
        AutoLock<PthreadMutex> al(taglock);
        readerlist.push_back(q);
    }
    void Node::AddWriter(weak_ptr<QueueBase> q) {
        AutoLock<PthreadMutex> al(taglock);
        writerlist.push_back(q);
    }

    void SignalReader(shared_ptr<QueueBase> q) {
        q->SignalReaderTagChanged();
    }

    void SignalWriter(shared_ptr<QueueBase> q) {
        q->SignalWriterTagChanged();
    }

    typedef std::list<weak_ptr<QueueBase> > QueueList;
    void GetQueues(QueueList &qlist, std::list<shared_ptr<QueueBase> > &out) {
        QueueList::iterator itr, end;
        itr = qlist.begin();
        end = qlist.end();
        while (itr != end) {
            shared_ptr<QueueBase> q = itr->lock();
            if (q) {
                out.push_back(q);
                ++itr;
            } else {
                itr = qlist.erase(itr);
            }
        }
    }

    void Node::SignalTagChanged() {
        std::list<shared_ptr<QueueBase> > readers;
        std::list<shared_ptr<QueueBase> > writers;
        {
            AutoLock<PthreadMutex> al(taglock);
            GetQueues(readerlist, readers);
            GetQueues(writerlist, writers);
        }
        std::for_each(readers.begin(), readers.end(), &SignalReader);
        std::for_each(writers.begin(), writers.end(), &SignalWriter);
    }

}

