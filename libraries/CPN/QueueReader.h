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
#include <string>

namespace CPN {

    /**
     * \brief Definition of the reader portion of the CPN queue class.
     */
    class CPN_API QueueReader {
    public:

        QueueReader(QueueReleaser *n, shared_ptr<QueueBase> q);

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
        const void* GetRawDequeuePtr(unsigned thresh, unsigned chan=0) {
            return queue->GetRawDequeuePtr(thresh, chan);
        }

        /**
         * This function is used to remove elements from the queue.
         * count elements will be removed from the queue when this function is
         * called.
         * \param count the number of bytes to remove from the queue
         * \invariant count <= thresh from GetRawDequeuePtr
         */
        void Dequeue(unsigned count) { queue->Dequeue(count); }

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
                unsigned numChans, unsigned chanStride) {
            return queue->RawDequeue(data, count, numChans, chanStride);
        }

        /**
         * A version of RawDequeue to use when there is only 1 channel.
         * \param data the data to enqueue
         * \param count the number of bytes to enqueue
         * \return true on success or false if the writer has disconnected and
         * there is not enough data to fill the request.
         */
        bool RawDequeue(void *data, unsigned count) { return queue->RawDequeue(data, count); }

        /**
         * \return the number of channels supported by this queue.
         */
        unsigned NumChannels() const { return queue->NumChannels(); }

        /**
         * \return the maximum threshold this queue supports.
         */
        unsigned MaxThreshold() const { return queue->MaxThreshold(); }

        unsigned QueueLength() const { return queue->QueueLength(); }
        /**
         * An accessor method for the number of elements currently in
         * the queue.
         *
         * \warning This function violates the rules of CPN.
         *
         * \return the number of elements in the queue.
         */
        unsigned Count() const { return queue->Count(); }

        /**
         * \warning This function violates the rules of CPN.
         * \return true if the queue is empty
         */
        bool Empty() const { return queue->Empty(); }

        /**
         * \return the typename for this queue
         */
        const std::string &GetDatatype() const { return queue->GetDatatype(); }

        /**
         * \return The current channel stride, the returned value is only
         * guaranteed to be consistent when called between calls to GetRawDequeuePtr
         * and Dequeue.
         */
        unsigned ChannelStride() const { return queue->DequeueChannelStride(); }

        /**
         * \return the key associated with this endpoint
         */
        Key_t GetKey() const { return queue->GetReaderKey(); }

        /**
         * Release the reader and reclame 
         * resources, all futher operations are invalid.
         */
        void Release();

        /** \brief Called by the node */
        void NotifyTerminate() { queue->NotifyTerminate(); }

        shared_ptr<QueueBase> GetQueue() { return queue; }
    private:
        QueueReleaser *releaser;
        shared_ptr<QueueBase> queue;
    };
}
#endif
