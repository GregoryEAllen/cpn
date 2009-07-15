/** \file
 * \brief Definition and implementation of the QueueInfo class.
 */
#ifndef CPN_QUEUEINFO_H
#define CPN_QUEUEINFO_H

namespace CPN {
	class Kernel;
	class NodeQueueReader;
	class NodeQueueWriter;
	class QueueFactory;
	class QueueBase;
	class QueueAttr;

	/**
	 * \brief Helper queue container object for the Kernel.
	 *
	 * \note This class is only used by the Kernel,
	 * NodeQueueReader, NodeQueueWriter, and NodeInfo.
	 */
	class QueueInfo {
	public:
		QueueInfo(Kernel *ker, const QueueAttr &attr);
		
		~QueueInfo();

		/**
		 * \return the queue this QueueInfo holds
		 */
		QueueBase* GetQueue(void) const { return queue; }

		/**
		 * Set the reader object. This function also registers
		 * the event with the queue.
		 * \param reader_ the reader to register
		 */
		void SetReader(NodeQueueReader* reader_);

		/**
		 * Clear the current reader
		 * \param checkdeath whether this clear indicates that
		 * this the endpoint that we are connected to is done.
		 */
		void ClearReader(bool checkdeath);

		/**
		 * \return the currently registered reader or 0 if none.
		 */
		NodeQueueReader* GetReader(void) const { return reader; }

		/**
		 * Set the writer object. This functio nalso registers
		 * the event with the queue.
		 * \param writer_ the writer to register
		 */
		void SetWriter(NodeQueueWriter* writer_);

		/**
		 * Clear the current writer.
		 * \param checkdeath whether this clear indicates that
		 * this the endpoint that we are connected to is done.
		 */
		void ClearWriter(bool checkdeath);

		/**
		 * \return the curretnly registered writer or 0 if none
		 */
		NodeQueueWriter* GetWriter(void) const { return writer; }

	private:
		/**
		 * Check our internal state to see if we are dead and if
		 * so call Kernel::QueueShutdown
		 */
		void CheckDeath(void);

		Kernel* kernel;
		QueueFactory* factory;
		QueueBase* queue;
		NodeQueueReader* reader;
		NodeQueueWriter* writer;
		bool readerset, writerset;
	};
}

#endif

