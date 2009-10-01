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
 * \brief The definition of reader end of the queue.
 * \author John Bridgman
 */

#ifndef CPN_QUEUEREADER_H
#define CPN_QUEUEREADER_H
#pragma once

#include "CPNCommon.h"
#include "QueueBase.h"
#include "MessageQueue.h"
#include "NodeMessage.h"
#include "QueueBlocker.h"
#include <string>

namespace CPN {

    /**
     * \brief Definition of the reader portion of the CPN queue class.
     */
    class CPN_API QueueReader
        : public std::tr1::enable_shared_from_this<QueueReader> {
    public:

        QueueReader(QueueBlocker *n, Key_t k);

        ~QueueReader();

        /**
         * Get a pointer to a buffer containing elements.
         *
         * \note access to the memory locations pointed to by the
         * returned pointer after Dequeue has been called is undefined.
         *
         * \param thresh the number of bytes to get
         * \param chan the channel to use
         * \return A void* to a block of memory thresh bytes long
         * blocks until threshold bytes available
         * 0 if the writer has disconnected and there is not enough
         * data to fill the request.
         */
        const void* GetRawDequeuePtr(unsigned thresh, unsigned
                chan=0);

        /**
         * This function is used to remove elements from the queue.
         * count elements will be removed from the queue when this function is
         * called.
         * \param count the number of bytes to remove from the queue
         * \invariant count <= thresh from GetRawDequeuePtr
         */
        void Dequeue(unsigned count);

        /**
         * Dequeue data from the queue directly into the memory pointed to by
         * data. This function shall be equivalent to
         * a call to GetRawDequeuePtr then a memcpy and then a call to Dequeue.
         *
         * \param data poiner to memory to dequeue to
         * \param count the number of bytes to copy into data
         * \param numChans the number of channels to write to
         * \param chanStride the distance in bytes between the beginning of
         * the channels in data.
         * \return true on success or false if the writer has disconnected and
         * there is not enough data to fill the request.
         */
        bool RawDequeue(void *data, unsigned count,
                unsigned numChans, unsigned chanStride);

        /**
         * A version of RawDequeue to use when there is only 1 channel.
         * \param data the data to enqueue
         * \param count the number of bytes to enqueue
         * \return true on success or false if the writer has disconnected and
         * there is not enough data to fill the request.
         */
        bool RawDequeue(void *data, unsigned count);

        /**
         * \return the number of channels supported by this queue.
         */
        unsigned NumChannels() const { CheckQueue(); return queue->NumChannels(); }

        /**
         * \return the maximum threshold this queue supports.
         */
         unsigned MaxThreshold() const { CheckQueue(); return queue->MaxThreshold(); }

        /**
         * An accessor method for the number of elements currently in
         * the queue.
         *
         * \warning This function violates the rules of CPN.
         *
         * \return the number of elements in the queue.
         */
        unsigned Count() const { CheckQueue(); return queue->Count(); }

        /**
         * \warning This function violates the rules of CPN.
         * \return true if the queue is empty
         */
        bool Empty() const { CheckQueue(); return queue->Empty(); }

        /**
         * \return the typename for this queue
         */
        const std::string &GetTypeName() const { return datatype; }

        /**
         * \return the key associated with this endpoint
         */
        Key_t GetKey() const { return key; }

        /**
         * \return the message queue to send message to the other
         * endpoint
         */
        shared_ptr<MsgPut<NodeMessagePtr> > GetMsgPut() { return upstream; }
        /**
         * Set the queue for this endpoint.
         * \param q the queue to set
         */
        void SetQueue(shared_ptr<QueueBase> q);
        /**
         * Send a message to the other endpoint.
         * \param msg the message to send
         */
        void PutMsg(NodeMessagePtr msg) { upstream->Put(msg); }
        /**
         * Shutdown this endpoint. Sends a shutdown message
         * to the other side. One may finish reading
         * data from the reader.
         */
        void Shutdown();
        /**
         * Release the reader and reclame 
         * resources, all futher operations are invalid.
         */
        void Release();
    private:
        void CheckQueue() const {
            blocker->CheckTerminate();
            while (!queue) blocker->ReadNeedQueue(key);
        }

        QueueBlocker *blocker;
        Key_t key;
        /// msgs from the writer
        mutable shared_ptr<MsgMutator<NodeMessagePtr, KeyMutator> > downstream;
        /// msgs to the writer
        mutable shared_ptr<MsgChain<NodeMessagePtr> > upstream;
        mutable shared_ptr<QueueBase> queue;
        std::string datatype;
        bool shutdown;
    };
}
#endif
