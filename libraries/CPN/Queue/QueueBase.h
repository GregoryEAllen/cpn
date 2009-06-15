/** \file
 * \brief Top Representations of generic queues for the CPN library.
 */
#ifndef CPN_QUEUEBASE_H
#define CPN_QUEUEBASE_H

#include "common.h"

namespace ::CPN {
	/**
	 * The base class for all queues in the CPN library.
	 */
	class QueueBase {
	public:
		QueueBase(QueueID id) : queueID(id) {}

		virtual ~QueueBase() {}

		/**
		 * Return the ID for this queue
		 * \return QueueID the unique identifier for this queue.
		 */
		QueueID getID(void) const { return queueID; }

		virtual QueueWriter getWriter() = 0;
		virtual QueueReader getReader() = 0;

		/**
		 * Get the total number of elements enqueued over the 
		 * lifetime of this queue.
		 * \return the number of elements.
		 */
		virtual ulong ElementsEnqueued(void) const = 0;

		/**
		 * Get the total number of elements dequeued over the lifetime
		 * of this queue.
		 * \return the number of elements.
		 */
		virtual ulong ElementsDequeued(void) const = 0;

	private:
		QueueID queueID;

	};

}

#endif
