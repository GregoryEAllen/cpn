/** \file
 * \brief BlockingQueueReader
 */
#ifndef CPN_BLOCKINGQUEUEREADER_H
#define CPN_BLOCKINGQUEUEREADER_H

#include "common.h"
#include "NodeQueueReader.h"
#include "QueueInfo.h"
#include "QueueStatus.h"

namespace CPN {

	class NodeInfo;

	/**
	 * \brief Simple blocking implementation of the NodeQueueReader.
	 *
	 */
	class BlockingQueueReader : public NodeQueueReader {
	public:
		BlockingQueueReader(NodeInfo* nodeinfo, const std::string &portname);

		~BlockingQueueReader();

		// From QueueReader
		const void* GetRawDequeuePtr(ulong thresh, ulong chan=0);

		void Dequeue(ulong count);

		bool RawDequeue(void* data, ulong count, ulong chan=0);

		ulong NumChannels(void) const;

		ulong Count(void) const;

		bool Empty(void) const;
	};
}

#endif

