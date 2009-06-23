/** \file
 * Definition for the Queue writer inteface.
 */

#ifndef CPN_QUEUEWRITER_H
#define CPN_QUEUEWRITER_H

#include "common.h"

namespace CPN {

	/**
	 * \brief Definition of the writer portion of the CPN queue class.
	 * \note There is no requirement on this interface be reentrant or
	 * synchronized. That is to say an instance of this class should be
	 * accessed by only one thread.
	 */
	class QueueWriter {
	public:

		virtual ~QueueWriter() {}

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
		virtual void* GetRawEnqueuePtr(ulong thresh, ulong chan=0) = 0;
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
		virtual void Enqueue(ulong count) = 0;

		/**
		 * This function shall be equivalent to
		 * <code>
		 * void* dest = q->GetRawEnqueuePtr(count, chan);
		 * if (!dest) return false;
		 * memcpy(dest, data, count);
		 * q->Enqueue(count);
		 * return true;
		 * </code>
		 *
		 * The underlying implementatin may implement ether the
		 * GetRawEnqueuePtr and Enqueue or RawEnqueue and
		 * then implement the other in terms of the one implemented.
		 *
		 * This function does not make sense when there is more than
		 * one channel.
		 *
		 * \param data pointer to the memory to enqueue
		 * \param count the number of bytes to enqueue
		 * \param chan the channel to enqueue
		 * \return true on success false if not enough space available
		 */
		virtual bool RawEnqueue(void* data, ulong count, ulong chan=0) = 0;

		/**
		 * \return the number of channels supported by this queue.
		 */
		virtual ulong NumChannels(void) const = 0;

		/**
		 * Get the space available in elements.
		 * \warning This function violates the rules of CPN.
		 * \return the number of bytes we can add to the queue without
		 * blocking.
		 */
		virtual ulong Freespace(void) const = 0;

		/**
		 * Test if the queue is currently full.
		 * \warning This function violates the rules of CPN.
		 * \return true if the queue is full, false otherwise
		 */
		virtual bool Full(void) const = 0;
	};

}
#endif
