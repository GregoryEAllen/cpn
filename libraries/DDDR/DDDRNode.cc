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
#include "DDDRNode.h"
#include "AutoLock.h"
#include "AutoUnlock.h"
#include "Assert.h"
#include <algorithm>
#include <functional>

namespace DDDR {

    Node::Node(unsigned long long key)
        : publicTag(key),
        privateTag(key),
        numblockees(0)
    {
    }

    Node::~Node() {}

    void Node::Block(const Tag &t, unsigned qsize) {
        Sync::AutoLock<Node> al(*this);
        privateTag.Count(std::max(privateTag.Count(), t.Count()) + 1);
        privateTag.QueueSize(qsize);
        publicTag = privateTag;
        std::deque<Queue*> b;
        blockees.swap(b);
        Tag tag = publicTag;
        al.Unlock();
        std::for_each(b.begin(), b.end(), std::bind2nd(std::mem_fun(&Queue::TagChanged), tag));
    }

    bool Node::Transmit(const Tag &t) {
        Sync::AutoLock<Node> al(*this);
        if (t == publicTag) {
            // Detect
            if (publicTag.QueueSize() == privateTag.QueueSize()) {
                return true;
            }
        }
        if (publicTag < t) {
            publicTag = t;
            publicTag.QueueSize(std::min(publicTag.QueueSize(), privateTag.QueueSize()));
            std::deque<Queue*> b;
            blockees.swap(b);
            Tag tag = publicTag;
            al.Unlock();
            std::for_each(b.begin(), b.end(), std::bind2nd(std::mem_fun(&Queue::TagChanged), tag));
        }
        return false;
    }

    void Node::Activate() {
        Sync::AutoLock<Node> al(*this);
        if (numblockees == 0) {
            publicTag.Reset();
            privateTag.Reset();
        }
    }

    Tag Node::GetPublicTag() const {
        Sync::AutoLock<Node> al(*this);
        return publicTag;
    }

    Tag Node::GetPrivateTag() const {
        Sync::AutoLock<Node> al(*this);
        return privateTag;
    }

    unsigned Node::GetNumBlockees() const {
        Sync::AutoLock<Node> al(*this);
        return numblockees;
    }

    void Node::RegisterTagChangeNotification(Queue *q, const Tag &t) {
        Sync::AutoLock<Node> al(*this);
        if (t == publicTag) {
            blockees.push_back(q);
        } else {
            Tag tag = publicTag;
            al.Unlock();
            q->TagChanged(tag);
        }
    }

    void Node::AddBlockee() {
        Sync::AutoLock<Node> al(*this);
        numblockees++;
    }

    void Node::RemBlockee() {
        Sync::AutoLock<Node> al(*this);
        numblockees--;
    }

    void Node::Lock() const {}

    void Node::Unlock() const {}

    Queue::Queue()
        : readblocked(false),
        writeblocked(false),
        writer(0),
        reader(0)
    {
    }

    Queue::~Queue() {}

    void Queue::SetReaderNode(Node *n) {
        reader = n;
    }

    void Queue::SetWriterNode(Node *n) {
        writer = n;
    }

    void Queue::TagChanged(const Tag t) {
        Sync::AutoLock<Queue> al(*this);
        bool blocked = false;
        Node *blockee, *blocker;
        if (readblocked && writeblocked) {
            al.Unlock();
            // self loop
            Detect();
        } else if (readblocked) {
            blocked = true;
            blockee = reader;
            blocker = writer;
        } else if (writeblocked) {
            blocked = true;
            blockee = writer;
            blocker = reader;
        }
        if (blocked) {
            al.Unlock();
            if (blockee->Transmit(t)) {
                Detect();
            } else {
                blocker->RegisterTagChangeNotification(this, t);
            }
        }
    }

    void Queue::ReadBlock() {
        readblocked = true;
        Node *blockee = reader;
        Node *blocker = writer;
        blocker->AddBlockee();
        Tag t = blocker->GetPublicTag();
        {
            AutoUnlock<Queue> aul(*this);
            blockee->Block(t, -1);
            blocker->RegisterTagChangeNotification(this, t);
        }
    }

    void Queue::WriteBlock(unsigned qsize) {
        writeblocked = true;
        Node *blockee = writer;
        Node *blocker = reader;
        blocker->AddBlockee();
        Tag t = blocker->GetPublicTag();
        {
            AutoUnlock<Queue> aul(*this);
            blockee->Block(t, qsize);
            blocker->RegisterTagChangeNotification(this, t);
        }
    }

    void Queue::ReadUnblock() {
        ASSERT(readblocked);
        readblocked = false;
        writer->RemBlockee();
        reader->Activate();
    }

    void Queue::WriteUnblock() {
        ASSERT(writeblocked);
        writeblocked = false;
        reader->RemBlockee();
        writer->Activate();
    }

    void Queue::Lock() const {}

    void Queue::Unlock() const {}
}

