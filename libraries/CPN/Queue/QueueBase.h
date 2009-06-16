/** \file
 * \brief Top Representations of generic queues for the CPN library.
 */
#ifndef CPN_QUEUEBASE_H
#define CPN_QUEUEBASE_H

#include "common.h"
#include "QueueAttr.h"

namespace ::CPN {
	/**
	 * The base class for all queues in the CPN library.
	 */
	class QueueBase {
	public:
		QueueBase(const QueueAttr &qattr) : qattr(qattr) {}

		virtual ~QueueBase() {}

		/**
		 * \return the QueueAttr for this queue.
		 */
		const QueueAttr &GetAttr(void) const { return qattr; }

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
		const QueueAttr qattr;
	};

}

#endif
