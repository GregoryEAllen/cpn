/** \file
 * \brief Definition and implementation of the QueueInfo class.
 */
#ifndef CPN_QUEUEINFO_H
#define CPN_QUEUEINFO_H

namespace CPN {
	class NodeQueueReader;
	class NodequeueWriter;

	class QueueInfo {
	public:
		QueueInfo(const QueueAttr &attr);
		
		~QueueInfo();

		QueueBase* GetQueue(void) const { return queue; }

		void SetReader(NodeQueueReader* reader_);

		NodeQueueReader* GetReader(void) const { return reader; }

		void SetWriter(NodeQueueWriter* writer_);

		NodeQueueWriter* GetWriter(void) const { return writer; }

	private:
		QueueFactory* factory;
		QueueBase* queue;
		NodeQueueReader* reader;
		NodeQueueWriter* writer;
	};
}

#endif

