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
#ifndef D4R_NODE_H
#define D4R_NODE_H
#pragma once
#include "D4RTag.h"
#include <deque>
namespace D4R {

/*
 * Implementation notes
 *
 * In the D4R algorithm there are four state transitions.
 *
 * BLOCK:
 * This state transition happens when a node blocks on a queue.  There
 * are two nodes in the block, the one who is blocked and the one who is being
 * blocked on. I will call these the blockee and the blocker respectively in
 * this discussion.
 * 
 * The following will happen in this transition:
 *  1. the queue will be marked as blocked
 *  2. blockee will update the queue size in its private tag.
 *  3. all fields in the blockee's private tag are copied to the blockees public tag.
 *  4. a copy of the blockers public tag is obtained and a change notification is registered.
 *  5. the blockee's public and private count will be set to 1 plus the maximum of the current blockee's
 *  count and the blocker's count.
 *
 * TRANSMIT:
 * This state transition happens whenever a change notification happens and the
 * queue is marked blocked and the tags are NOT equal.
 *
 * The following happens in this transition:
 *  1. The blockee and blocker tags are compared. If the blockee tag is less than
 *  the blocker tag then proceed otherwise ignore the notification.
 *  2. Copy the count and nodekey of the public tag of the blocker to the public tag of the blockee
 *  and set the qsize of the blockee to the minimum of the qsize of the blocker and blockee.
 *
 * DETECT:
 * If the tags are equal and the blockee's private qsize and public qsize are equal then
 * we perform the DETECT step. This is where we resolve the deadlock.
 *
 * ACTIVATE:
 * Go back to normal activity.
 *
 */

    /*
     */

    class Node {
    public:

        Node(unsigned long long key);

        virtual ~Node();

        Tag GetTag() const;
        void Block(Tag t, unsigned qsize);
        bool Transmit(Tag t);

        virtual void SignalTagChanged() = 0;

        virtual void Lock() const = 0;
        virtual void Unlock() const = 0;
    protected:
        Tag publicTag;
        Tag privateTag;
    private:
        Node(const Node&);
        Node &operator=(const Node&);
    };

}
#endif
