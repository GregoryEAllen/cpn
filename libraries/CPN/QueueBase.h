/** \file
 * \brief Top Representations of generic queues for the CPN library.
 */
#ifndef CPN_QUEUEBASE_H
#define CPN_QUEUEBASE_H

#include "common.h"
#include "QueueAttr.h"
#include "QueueWriter.h"
#include "QueueReader.h"

class PthreadCondition;

namespace CPN {

	/**
	 * The base class for all queues in the CPN library.
	 */
	class QueueBase :
		public QueueReader,
		public QueueWriter
	{
	public:
		QueueBase(const QueueAttr &qattr);

		virtual ~QueueBase();

		/**
		 * \return the QueueAttr for this queue.
		 */
		virtual const QueueAttr &GetAttr(void) const { return qattr; }

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

		/*
		// Possible implementation for D4R message passing. 
		virtual void PutReaderMessage(void* data, ulong count);
		virtual bool GetReaderMessage(void* data, ulong count);
		virtual void PutWriterMessage(void* data, ulong count);
		virtual bool GetReaderMessage(void* data, ulong count);
		*/

		// Implementation for unblocking...
		virtual void RegisterReaderEvent(PthreadCondition* evt) = 0;
		virtual void RegisterWriterEvent(PthreadCondition* evt) = 0;

	private:
		const QueueAttr qattr;
	};

}

#endif
