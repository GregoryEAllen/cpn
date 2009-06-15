/** \file
 * The definition of reader end of the queue.
 */

#ifndef CPN_QUEUE_QUEUEREADER_H
#define CPN_QUEUE_QUEUEREADER_H
namespace CPN::Queue {
	/**
	 * \brief Definition of the reader portion of the CPN queue class.
	 * \note There is no requirement on this interface to be reentrant or
	 * synchronized. That is to say, an instance of this class should only
	 * be accessed from one thread.
	 */
	class QueueReader {
	public:
		/**
		 * Get a pointer to a buffer containing elements.  This
		 * function will block until there are thresh elements
		 * available.
		 *
		 * \note a call to this function without an accompanying call
		 * to Dequeue is undefined. 
		 * \note access to the memory locations pointed to by the
		 * returned pointer after Dequeue has been called is undefined.
		 * \note Two calls to this function back to back may return
		 * different pointers.
		 *
		 * \param thresh the number of elements to get \param chan the
		 * channel to use \return A void* to a block of memory
		 * containing thresh elements
		 */
		virtual const void* GetRawDequeuePtr(ulong thresh, ulong
				chan=0) = 0;

		/**
		 * This function is used to remove elements from the queue.
		 * count elements will be removed from the queue when this function is
		 * called.
		 * \param count the number of elements to remove from the queue
		 */
		virtual void Dequeue(ulong count) = 0;

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
		 * An accessor method for the number of elements currently in
		 * the queue.
		 *
		 * \warning This function violates the rules of CPN.
		 *
		 * \return the number of elements in the queue.
		 */
		virtual ulong Count(void) const = 0;

		/**
		 * \warning This function violates the rules of CPN.
		 * \return true if the queue is empty
		 */
		virtual bool Empty(void) const = 0;
		
	};
}
#endif
