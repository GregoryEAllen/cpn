/** \file
 * Definition for the Queue writer inteface.
 */

#ifndef CPN_QUEUE_QUEUEWRITER_H
#define CPN_QUEUE_QUEUEWRITER_H

namespace CPN::Queue {

	/**
	 * \brief Definition of the writer portion of the CPN queue class.
	 * \note There is no requirement on this interface be reentrant or
	 * synchronized. That is to say an instance of this class should be
	 * accessed by only one thread.
	 */
	class QueueWriter {
	public:

		/**
		 * Return a pointer to a buffer of memory that contains
		 * thresh entries that we can write into.
		 * This function will block until there are thresh entries
		 * available.
		 *
		 * \note A call to this function without an accompanying call to
		 * Enqueue is undefined.
		 * \note An access to the memory locations defined by the return
		 * value is undefined after a call to Enqueue.
		 *
		 * \param thresh the number entries we need in the returned buffer.
		 * \param chan the channel to use
		 * \return void* to the memory buffer
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
		 * \param count the number of entries to be placed in the buffer
		 * \invariant count <= thresh from GetRawEnqueuePtr
		 */
		virtual void Enqueue(ulong count) = 0;
		/**
		 * This function can be used instead of GetRawenqueuePtr and
		 * Enqueue if we already have the data available.
		 * This function will block untill data can be completely written
		 * to the queue.
		 *
		 * \note A call to this function between a call to GetRawEnqueuePtr
		 * and Enqueue is undefined.
		 *
		 * \param data a void* to the data to enqueue
		 * \param count the number of entries in the memory pointed to by
		 * data
		 * \param chan the channel to use
		 */
		virtual void RawEnqueue(void* data, ulong count, ulong chan=0) = 0;

		/**
		 * \return the number of channels supported by this queue.
		 */
		virtual ulong NumChannels(void) const = 0;

		/**
		 * The distance in memory in elements between channels.
		 * !!! Confirm
		 */
		virtual ulong ChannelStride(void) const = 0;

		/**
		 * Get the space available in elements.
		 * \warning This function violates the rules of CPN.
		 * \return the number of elements we can add to the queue without
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
