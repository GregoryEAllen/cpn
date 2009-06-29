/** \file
 * The definition of reader end of the queue.
 */

#ifndef CPN_QUEUEREADER_H
#define CPN_QUEUEREADER_H

#include "common.h"

namespace CPN {
	/**
	 * \brief Definition of the reader portion of the CPN queue class.
	 *
	 * \note There is no requirement on this interface to be reentrant or
	 * synchronized. That is to say, an instance of this class should only
	 * be accessed from one thread.
	 */
	class QueueReader {
	public:

		virtual ~QueueReader() {}

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
		virtual const void* GetRawDequeuePtr(ulong thresh, ulong
				chan=0) = 0;

		/**
		 * This function is used to remove elements from the queue.
		 * count elements will be removed from the queue when this function is
		 * called.
		 * \param count the number of bytes to remove from the queue
		 */
		virtual void Dequeue(ulong count) = 0;

		/**
		 * Dequeue data from the queue directly into the memory pointed to by
		 * data.
		 * This function shall be equivalent to
		 * a call to GetRawDequeuePtr then a memcpy and then a call to Dequeue.
		 *
		 * This function does not make sense when there is more
		 * than one channel.
		 *
		 * \param data poiner to memory to dequeue to
		 * \param count the number of bytes to copy into data
		 * \param chan the channel
		 * \return true on success false on failure
		 */
		virtual bool RawDequeue(void * data, ulong count, ulong chan = 0) = 0;

		/**
		 * \return the number of channels supported by this queue.
		 */
		virtual ulong NumChannels(void) const = 0;

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
