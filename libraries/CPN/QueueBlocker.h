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
 * \brief QueueBlocker is an abstract data type for
 * the endpoints to reduce coupling.
 * \author John Bridgman
 */

#ifndef CPN_QUEUEBLOCKER_H
#define CPN_QUEUEBLOCKER_H
#pragma once

#include "CPNCommon.h"
#include "MessageQueue.h"

namespace CPN {
    /**
     * Interface that the QueueReader and QueueWriter use to block through.
     */
    class QueueBlocker {
    public:
        virtual ~QueueBlocker() {}
        /**
         * Function called by the endpoint when the endpoint wants to block on read.
         * \param reader the endpoint
         * \param thresh the amount we are trying to aquire
         */
        virtual void ReadBlock(shared_ptr<QueueReader> reader, unsigned thresh) = 0;
        /**
         * Function called by the endpoint when teh endpoint wants to block on write.
         * \param writer the endpoint
         * \param thresh the amount we are trying to send
         */
        virtual void WriteBlock(shared_ptr<QueueWriter> writer, unsigned thresh) = 0;
        /**
         * Function called by the endpoint notifying that the
         * endpoint does not have the queue to read from.
         * \param key the key for this endpoint
         */
        virtual void ReadNeedQueue(Key_t key) = 0;
        /**
         * Function called by the endpoint nofifying that
         * the endpoint does not have the queue to write to.
         * \param key the key for this endpoint
         */
        virtual void WriteNeedQueue(Key_t key) = 0;
        /**
         * Function the endpoint calls when the
         * endpoint should be disposed of.
         * \param key the endpoint key
         */
        virtual void ReleaseWriter(Key_t key) = 0;
        /**
         * Function the endpoint calls when the
         * endpoint should be disposed of.
         * \param key the endpoint key
         */
        virtual void ReleaseReader(Key_t key) = 0;
        /**
         * Get the message queue that the endpoint should
         * forward to
         * \return a shared pointer to the message queue
         */
        virtual shared_ptr<MsgPut<NodeMessagePtr> > GetMsgPut() = 0;
        /**
         * This function should be called by the endpoint
         * periodically to check for termination.
         * \throw ShutdownException
         */
        virtual void CheckTerminate() = 0;
    };
}

#endif

