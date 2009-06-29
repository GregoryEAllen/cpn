/** \file
 * \brief BlockingQueueWriter
 */
#ifndef CPN_BLOCKINGQUEUEWRITER_H
#define CPN_BLOCKINGQUEUEWRITER_H

#include "common.h"
#include "NodeQueueWriter.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "QueueInfo.h"

namespace CPN {

	class NodeInfo;
	class QueueBase;

	/**
	 * \brief Simple blocking implementation of NodeQueueWriter.
	 *
	 * TODO: Fix the problem associated with setting a new
	 * queue inbetween calls to GetRawEnqueuePtr and Enqueue.
	 */
	class BlockingQueueWriter : public NodeQueueWriter {
	public:
		BlockingQueueWriter(NodeInfo* nodeinfo, const std::string &portname)
		       	: NodeQueueWriter(nodeinfo, portname), queueinfo(0), shutdown(false)
		{}

		~BlockingQueueWriter() {}

		// From QueueWriter
		void* GetRawEnqueuePtr(ulong thresh, ulong chan=0);

		void Enqueue(ulong count);

		bool RawEnqueue(void* data, ulong count, ulong chan=0);

		ulong NumChannels(void) const;

		ulong Freespace(void) const;

		bool Full(void) const;

		// From NodeQueueWriter
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

