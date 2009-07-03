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

	/**
	 * \brief The base class for all queues in the CPN library.
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
		const QueueAttr &GetAttr(void) const { return qattr; }

		void SetReaderStatusHandler(Sync::StatusHandler<QueueStatus>* rsh);
		void SetWriterStatusHandler(Sync::StatusHandler<QueueStatus>* wsh);

	protected:
		void NotifyReaderOfWrite(void);
		void NotifyWriterOfRead(void);
	private:
		void Notify(Sync::StatusHandler<QueueStatus>* stathand, QueueStatus newStatus);
		const QueueAttr qattr;
		PthreadMutex statusHandlerMutex;
		Sync::StatusHandler<QueueStatus>* readerStatusHandler;
		Sync::StatusHandler<QueueStatus>* writerStatusHandler;
	};

}

#endif
