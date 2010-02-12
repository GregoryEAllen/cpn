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

#if 1
#include <stdio.h>
#define DEBUG(fmt, ...) printf(fmt, ## __VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

namespace D4R {

    Node::Node(Key_t key)
        : publicTag(key),
        privateTag(key)
    {
    }

    Node::~Node() {}

    Tag Node::GetPublicTag() const {
        Sync::AutoLock<PthreadMutex> al(taglock);
        return publicTag;
    }

    void Node::SetPublicTag(Tag t) {
        Sync::AutoLock<PthreadMutex> al(taglock);
        publicTag = t;
    }

    Tag Node::GetPrivateTag() const {
        Sync::AutoLock<PthreadMutex> al(taglock);
        return privateTag;
    }

    void Node::SetPrivateTag(Tag t) {
        Sync::AutoLock<PthreadMutex> al(taglock);
        privateTag = t;
    }

    void Node::Block(Tag t, unsigned qsize) {
        Sync::AutoLock<PthreadMutex> al(taglock);
        privateTag.QueueSize(qsize);
        privateTag.Count(std::max(privateTag.Count(), t.Count()) + 1);
        DEBUG("Node %llu:%llu block %d\n", privateTag.Count(), privateTag.Key(), (int)privateTag.QueueSize());
        publicTag = privateTag;
        al.Unlock();
        SignalTagChanged();
    }

    bool Node::Transmit(Tag t) {
        Sync::AutoLock<PthreadMutex> al(taglock);
        if (publicTag < t) {
            DEBUG("Node %llu:%llu transfer %d : (%llu, %llu %d) < (%llu, %llu, %d)\n",
                    privateTag.Count(), privateTag.Key(), (int)privateTag.QueueSize(),
                    publicTag.Count(), publicTag.Key(), (int)publicTag.QueueSize(),
                    t.Count(), t.Key(), (int)t.QueueSize());
            unsigned qsize = std::min(publicTag.QueueSize(), t.QueueSize());
            publicTag = t;
            publicTag.QueueSize(qsize);
            al.Unlock();
            SignalTagChanged();
        } else if (publicTag == t) {
            DEBUG("Node %llu:%llu transfer %d : (%llu, %llu %d) == (%llu, %llu, %d)\n",
                    privateTag.Count(), privateTag.Key(), (int)privateTag.QueueSize(),
                    publicTag.Count(), publicTag.Key(), (int)publicTag.QueueSize(),
                    t.Count(), t.Key(), (int)t.QueueSize());
            bool detected = privateTag.QueueSize() == publicTag.QueueSize();
            al.Unlock();
            if (!detected) {
                SignalTagChanged();
            }
            return detected;
        } else {
            DEBUG("Node %llu:%llu transfer nop %d : (%llu, %llu %d) > (%llu, %llu, %d)\n",
                    privateTag.Count(), privateTag.Key(), (int)privateTag.QueueSize(),
                    publicTag.Count(), publicTag.Key(), (int)publicTag.QueueSize(),
                    t.Count(), t.Key(), (int)t.QueueSize());
        }
        return false;
    }

}

