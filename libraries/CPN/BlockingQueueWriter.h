/** \file
 * \brief BlockingQueueWriter
 */
#ifndef CPN_BLOCKINGQUEUEWRITER_H
#define CPN_BLOCKINGQUEUEWRITER_H

#include "common.h"
#include "NodeQueueWriter.h"
#include "QueueInfo.h"
#include "QueueStatus.h"
#include "ReentrantLock.h"
#include "StatusHandler.h"

namespace CPN {

	class NodeInfo;
	class QueueBase;

	/**
	 * \brief Simple blocking implementation of NodeQueueWriter.
	 *
	 */
	class BlockingQueueWriter : public NodeQueueWriter {
	public:
		BlockingQueueWriter(NodeInfo* nodeinfo, const std::string &portname);

		~BlockingQueueWriter();

		// From QueueWriter
		void* GetRawEnqueuePtr(ulong thresh, ulong chan=0);

		void Enqueue(ulong count);

		bool RawEnqueue(void* data, ulong count, ulong chan=0);

		ulong NumChannels(void) const;

		ulong Freespace(void) const;

		bool Full(void) const;

		// From NodeQueueWriter
		void SetQueueInfo(QueueInfo* queueinfo_);

		void ClearQueueInfo(void);

		Sync::StatusHandler<QueueStatus>* GetStatusHandler(void) { return &status; }

		QueueInfo* GetQueueInfo(void);

		void Terminate(void);

	private:
		QueueBase* CheckQueue(void) const;

		mutable Sync::ReentrantLock lock;
		QueueInfo* queueinfo;
		Sync::StatusHandler<QueueStatus> status;
	};
}

#endif

