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
#include "AutoLock.h"
#include "AutoUnlock.h"
#include "Assert.h"
#include <algorithm>
#include <functional>

#if 0
#include <stdio.h>
#define DEBUG(fmt, ...) printf(fmt, ## __VA_ARGS__)
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
        Sync::AutoLock<PthreadMutex> al(taglock);
        return publicTag;
    }

    void Node::SetPublicTag(const Tag &t) {
        Sync::AutoLock<PthreadMutex> al(taglock);
        publicTag = t;
    }

    Tag Node::GetPrivateTag() const {
        Sync::AutoLock<PthreadMutex> al(taglock);
        return privateTag;
    }

    void Node::SetPrivateTag(const Tag &t) {
        Sync::AutoLock<PthreadMutex> al(taglock);
        privateTag = t;
    }

    void Node::Block(const Tag &t, unsigned qsize) {
        Sync::AutoLock<PthreadMutex> al(taglock);
        privateTag.QueueSize(qsize);
        privateTag.Count(std::max(privateTag.Count(), t.Count()) + 1);
        DEBUG("Node %llu:%llu block %d\n", privateTag.Count(), privateTag.Key(), (int)privateTag.QueueSize());
        publicTag = privateTag;
        al.Unlock();
        SignalTagChanged();
    }

    bool Node::Transmit(const Tag &t) {
        Sync::AutoLock<PthreadMutex> al(taglock);
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

}

