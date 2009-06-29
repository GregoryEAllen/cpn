/** \file
 * \brief BlockingQueueReader
 */
#ifndef CPN_BLOCKINGQUEUEREADER_H
#define CPN_BLOCKINGQUEUEREADER_H

#include "common.h"
#include "NodeQueueReader.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "QueueInfo.h"

namespace CPN {

	class QueueBase;
	class NodeInfo;

	/**
	 * Simple blocking implementation of the NodeQueueReader.
	 * TODO: Fix the problem associated with setting a new queue
	 * inbetween calls to GetRawDequeuePtr and Dequeue.
	 */
	class BlockingQueueReader : public NodeQueueReader {
	public:
		BlockingQueueReader(NodeInfo* nodeinfo, const std::string &portname)
		       	: NodeQueueReader(nodeinfo, portname), queueinfo(0), shutdown(false)
	       {}

		~BlockingQueueReader() {}

		// From QueueReader
		const void* GetRawDequeuePtr(ulong thresh, ulong chan=0);

		void Dequeue(ulong count);

		bool RawDequeue(void * data, ulong count, ulong chan=0);

		ulong NumChannels(void) const;

		ulong Count(void) const;

		bool Empty(void) const;

		// From NodeQueueReader
		void SetQueueInfo(QueueInfo* queueinfo_);

		QueueInfo* GetQueueInfo(void);

		PthreadCondition* GetEvent(void) { return &event; }

		void Terminate(void);

	private:
		QueueBase* CheckQueue(void) const;
		mutable PthreadCondition event;
		mutable PthreadMutex lock;
		QueueInfo* queueinfo;
		bool shutdown;
	};
}

#endif

