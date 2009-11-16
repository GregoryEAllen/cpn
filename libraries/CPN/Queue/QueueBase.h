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
 * \brief Top Representations of generic queues for the CPN library.
 * \author John Bridgman
 */
#ifndef CPN_QUEUEBASE_H
#define CPN_QUEUEBASE_H

#include "CPNCommon.h"
#include "Message.h"

#include "ReentrantLock.h"

namespace CPN {

	/**
	 * \brief The base class for all queues in the CPN library.
	 */
	class CPN_LOCAL QueueBase 
    : protected ReaderMessageHandler, protected WriterMessageHandler
    {
	public:

		virtual ~QueueBase();

        /**
         * Get a pointer to a buffer containing elements.
         *
         * \note access to the memory locations pointed to by the
         * returned pointer after Dequeue has been called is undefined.
         *
         * \param thresh the number of bytes to get
         * \param chan the channel to use
         * \return A void* to a block of memory containing thresh bytes
         * or 0 if there are not thresh bytes available.
         */
        virtual const void* GetRawDequeuePtr(unsigned thresh, unsigned
                chan=0) = 0;

        /**
         * This function is used to remove elements from the queue.
         * count elements will be removed from the queue when this function is
         * called.
         * \param count the number of bytes to remove from the queue
         */
        virtual void Dequeue(unsigned count) = 0;

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
         * \return true on success false on failure
         */
        virtual bool RawDequeue(void* data, unsigned count,
                unsigned numChans, unsigned chanStride) = 0;

        /**
         * A version of RawDequeue to use when there is only 1 channel.
         * \param data the data to enqueue
         * \param count the number of bytes to enqueue
         * \return true on success false if there is not enough space
         */
        virtual bool RawDequeue(void* data, unsigned count) = 0;

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
		 * \return void* to the memory buffer, 0 if not enough space available
		 */
		virtual void* GetRawEnqueuePtr(unsigned thresh, unsigned chan=0) = 0;

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
		virtual void Enqueue(unsigned count) = 0;

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
		 * \return true on success false if not enough space available
		 */
		virtual bool RawEnqueue(const void* data, unsigned count,
                unsigned numChans, unsigned chanStride) = 0;

        /**
         * A version of RawEnqueue to use when there is only 1 channel.
         * \param data pointer to the memory to enqueue
         * \param count the number of bytes to enqueue
         * \return true on success false if not enough space available
         */
		virtual bool RawEnqueue(const void* data, unsigned count) = 0;


        /**
         * \return the number of channels supported by this queue.
         */
        virtual unsigned NumChannels() const = 0;

        /**
         * \return the number of bytes in the queue.
         */
        virtual unsigned Count() const = 0;

        /**
         * \return true if the queue is empty
         */
        virtual bool Empty() const = 0;

		/**
		 * \return the number of bytes we can add to the queue without
		 * blocking.
		 */
		virtual unsigned Freespace() const = 0;

		/**
		 * \return true if the queue is full, false otherwise
		 */
		virtual bool Full() const = 0;

        /**
         * \return the maximum threshold this queue supports
         * in bytes
         */
        virtual unsigned MaxThreshold() const = 0;

        /**
         * \return the maximum number of bytes that can be
         * put in this queue.
         */
        virtual unsigned QueueLength() const = 0;

        /**
         * Ensure that this queue has at least queueLen bytes
         * of space and can suport at least maxThresh as the maxThreshold
         * the new queue length will be max(queueLen, QueueLength())
         * and the new max threshold will be max(maxThresh, MaxThreshold())
         * \param queueLen the next queue length
         * \param maxThresh the next max threshold
         */
        virtual void Grow(unsigned queueLen, unsigned maxThresh) = 0;

        void SetDatatype(const std::string &type) { datatype = type; }

        const std::string &GetDatatype() const { return datatype; }

        void SetReaderMessageHandler(ReaderMessageHandler *rmhan);

        ReaderMessageHandler *GetReaderMessageHandler();

        void ClearReaderMessageHandler();

        void SetWriterMessageHandler(WriterMessageHandler *wmhan);

        WriterMessageHandler *GetWriterMessageHandler();

        void ClearWriterMessageHandler();

        const Sync::ReentrantLock &GetLock() const { return lock; }

	protected:
		QueueBase();
        Sync::ReentrantLock lock;
        Sync::ReentrantCondition cond;
        bool CheckRMH();
        bool CheckWMH();
        virtual void LogState() {}
	private:
        bool shutdown;
        std::string datatype;
	};

}

#endif
