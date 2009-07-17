/** \file
 * \brief Top Representations of generic queues for the CPN library.
 */
#ifndef CPN_QUEUEBASE_H
#define CPN_QUEUEBASE_H

#include "common.h"
#include "QueueAttr.h"
#include "QueueWriter.h"
#include "QueueReader.h"
#include "StatusHandler.h"
#include "QueueStatus.h"
#include "PthreadMutex.h"

namespace CPN {

	class QueueInfo;
	class QueueDatatype;

	/**
	 * \brief The base class for all queues in the CPN library.
	 */
	class QueueBase :
		public QueueReader,
		public QueueWriter
	{
	public:
		QueueBase(const QueueAttr &attr);

		virtual ~QueueBase();

		/**
		 * \return the QueueAttr for this queue.
		 */
		const QueueAttr &GetAttr(void) const { return attr; }

		/**
		 * \return the queue name
		 */
		const std::string GetName(void) const { return attr.GetName(); }

		const QueueDatatype* GetDatatype(void) const { return attr.GetDatatype(); }

	protected:
		/**
		 * Function to notify the reader of a write operation.
		 * This function should be called by implementations
		 * when write is success.
		 */
		void NotifyReaderOfWrite(void);
		/**
		 * Function to notify the writer of a read operation.
		 * This function should be called by implementations
		 * when a read is success.
		 */
		void NotifyWriterOfRead(void);

		QueueAttr attr;
	private:
		/**
		 * Send a notification currently to the given status handler.
		 * \param stathand pointer to status handler
		 * \param newStatus the status to notify
		 */
		void Notify(Sync::StatusHandler<QueueStatus>* stathand, QueueStatus newStatus);

		/**
		 * Set the status handler to use for this queue.
		 *
		 * \param rsh pointer to the status handler.
		 */
		void SetReaderStatusHandler(Sync::StatusHandler<QueueStatus>* rsh);

		/**
		 * Clear the status handler for reading.
		 */
		void ClearReaderStatusHandler(void);

		/**
		 * Set the status handler for writing for this queue.
		 * \param wsh pointer to the status handler
		 */
		void SetWriterStatusHandler(Sync::StatusHandler<QueueStatus>* wsh);

		/**
		 * Clear the writer status handler.
		 */
		void ClearWriterStatusHandler(void);


		PthreadMutex statusHandlerMutex;
		Sync::StatusHandler<QueueStatus>* readerStatusHandler;
		Sync::StatusHandler<QueueStatus>* writerStatusHandler;

		friend class QueueInfo;
	};

}

#endif
