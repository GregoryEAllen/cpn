/** \file
 * \brief BlockingQueueWriter
 */
#ifndef CPN_BLOCKINGQUEUEWRITER_H
#define CPN_BLOCKINGQUEUEWRITER_H

#include "NodeQueueWriter.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "QueueInfo.h"

namespace CPN {

	class NodeInfo;
	class QueueBase;

	class BlockingQueueWriter : public NodeQueueWriter {
	public:
		BlockingQueueWriter(NodeInfo* nodeinfo, const std::string &portname)
		       	: NodeQueueWriter(nodeinfo, portname), queueinfo(0) {}

		~BlockingQueueWriter() { SetQueue(0); }

		// From QueueWriter
		void* GetRawEnqueuePtr(ulong thresh, ulong chan=0);

		void Enqueue(ulong count);

		bool RawEnqueue(void* data, ulong count, ulong chan=0);

		ulong NumChannels(void) const;

		ulong Freespace(void) const;

		bool Full(void) const;

		// From NodeQueueWriter
		void SetQueue(QueueInfo* queueinfo_);

		QueueInfo* GetQueue(void);

		PthreadCondition* GetEvent(void) { return &event; }

	private:
		QueueBase* CheckQueue(void) const {
			while (!queueinfo) event.Wait(lock);
			return queueinfo->GetQueue();
		};

		mutable PthreadCondition event;
		mutable PthreadMutex lock;
		QueueInfo* queueinfo;
	};
}

#endif

