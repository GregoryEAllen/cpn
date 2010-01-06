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
 * \brief Definition for the Queue writer inteface.
 * \author John Bridgman
 */

#ifndef CPN_QUEUEWRITER_H
#define CPN_QUEUEWRITER_H
#pragma once

#include "CPNCommon.h"
#include "QueueBase.h"
#include <string>

namespace CPN {

	/**
	 * \brief Definition of the writer portion of the CPN queue class.
	 */
	class CPN_API QueueWriter {
	public:

        QueueWriter(QueueReleaser *qr, shared_ptr<QueueBase>);

		~QueueWriter();

		/**
		 * Return a pointer to a buffer of memory that contains
		 * thresh entries that we can write into.
		 *
		 * \note A call to this function without an accompanying call to
		 * Enqueue is undefined.
		 * \note An access to the memory locations defined by the return
		 * value is undefined after a call to Enqueue.
		 *
		 * \param thresh the number bytes we need in the returned buffer.
		 * \param chan the channel to use
		 * \return void* to a block of memory at least thresh bytes long,
         * blocks until thresh bytes available
         * \throws BrokenQueueException if the reader is released
		 */
		void* GetRawEnqueuePtr(unsigned thresh, unsigned chan=0) {
            return queue->GetRawEnqueuePtr(thresh, chan);
        }

		/**
		 * This function is used to release the buffer obtained with
		 * GetRawEnqueuePtr. The count specifies the number of 
		 * entries that we want to be placed in the buffer.
		 *
		 * \note A call to this function without an accompanying call
		 * to GetRawEnqueuePtr is undefined.
		 *
		 * \param count the number of bytes to be placed in the buffer
		 * \invariant count <= thresh from GetRawEnqueuePtr
		 */
		void Enqueue(unsigned count) { queue->Enqueue(count); }

		/**
		 * This function shall be equivalent to
		 * a call to GetRqwEnqueuePtr and a memcpy and then
		 * a call to Enqueue
		 *
		 * The underlying implementatin may implement ether the
		 * GetRawEnqueuePtr and Enqueue or RawEnqueue and
		 * then implement the other in terms of the one implemented.
		 *
		 * \param data pointer to the memory to enqueue
		 * \param count the number of bytes to enqueue
         * \param numChans the number of channels to write to
         * \param chanStride the distance in bytes between the beginning of
         * the channels in data.
         * \throws BrokenQueueException if the reader is released
		 */
		void RawEnqueue(void *data, unsigned count,
                unsigned numChans, unsigned chanStride) {
            queue->RawEnqueue(data, count, numChans, chanStride);
        }

        /**
         * A version of RawEnqueue to use when there is only 1 channel.
         * \param data pointer to the memory to enqueue
         * \param count the number of bytes to enqueue
         * \throws BrokenQueueException if the reader is released
         */
		void RawEnqueue(void *data, unsigned count) { queue->RawEnqueue(data, count); }

		/**
		 * \return the number of channels supported by this queue.
		 */
		unsigned NumChannels() const { return queue->NumChannels(); }

        /**
         * \return the maximum threshold this queue supports.
         */
        unsigned MaxThreshold() const { return queue->MaxThreshold(); }

		/**
		 * Get the space available in elements.
		 * \warning This function violates the rules of CPN.
		 * \return the number of bytes we can add to the queue without
		 * blocking.
		 */
		unsigned Freespace() const { return queue->Freespace(); }

		/**
		 * Test if the queue is currently full.
		 * \warning This function violates the rules of CPN.
		 * \return true if the queue is full, false otherwise
		 */
		bool Full() const { return queue->Full(); }

		/**
		 * \return the typename for this queue
		 */
		const std::string &GetDatatype() const { return queue->GetDatatype(); }

        /**
         * \return the key associated with this endpoint
         */
        Key_t GetKey() const { return queue->GetWriterKey(); }

        /**
         * Release this queue, will start releasing the resources
         * used.
         */
        void Release();
    private:
        QueueReleaser *releaser;
        shared_ptr<QueueBase> queue;
	};

}
#endif
