/** \file
 * \brief BlockingQueueReader
 */
#ifndef CPN_BLOCKINGQUEUEREADER_H
#define CPN_BLOCKINGQUEUEREADER_H

#include "NodeQueueReader.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"

namespace CPN {

	class QueueInfo;
	class QueueBase;
	class NodeInfo;

	class BlockingQueueReader : public NodeQueueReader {
	public:
		BlockingQueueReader(const NodeInfo* nodeinfo, const std::string &portname)
		       	: NodeQueueReader(nodeinfo, portname), queueinfo(0) {}

		~BlockingQueueReader() { SetQueue(0); }

		// From QueueReader
		const void* GetRawDequeuePtr(ulong thresh, ulong chan=0);

		void Dequeue(ulong count);

		bool RawDequeue(void * data, ulong count, ulong chan=0);

		ulong NumChannels(void) const;

		ulong Count(void) const;

		bool Empty(void) const;

		// From NodeQueueReader
		void SetQueue(QueueInfo* queueinfo_);

		QueueInfo* GetQueue(void);

		PthreadCondition* GetEvent(void) { return event; }

	private:
		QueueBase* CheckQueue(void) const {
			while (!queueinfo) event.Wait(lock);
			return queueinfo->GetQueue();
		}
		mutable PthreadCondition event;
		mutable PthreadMutex lock;
		QueueInfo* queueinfo;
	};
}

#endif

