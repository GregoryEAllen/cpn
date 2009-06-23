/** \file
 */

#ifndef CPN_NODEQUEUEREADER_H
#define CPN_NODEQUEUEREADER_H

#include "QueueReader.h"

namespace CPN {
	class QueueInfo;
	class NodeInfo;

	class NodeQueueReader : public QueueReader {
	public:
		NodeQueueReader(const NodeInfo* nodeinfo_, const std::string &portname_)
			: nodeinfo(nodeinfo_), portname(portname_) {}

		virtual void SetQueue(QueueInfo* queueinfo_) = 0;

		virtual QueueInfo* GetQueue(void) = 0;

		virtual PthreadCondition* GetEvent(void) = 0;

		NodeInfo* GetNodeInfo(void) const { return nodeinfo; }

		const std::string &GetPortName(void) const { return portname; }
	private:
		const std::string portname;
		const NodeInfo* nodeinfo;
	};
}

#endif
