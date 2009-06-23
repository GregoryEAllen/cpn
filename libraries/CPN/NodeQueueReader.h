/** \file
 */

#ifndef CPN_NODEQUEUEREADER_H
#define CPN_NODEQUEUEREADER_H

#include "QueueReader.h"
#include <string>

class PthreadCondition;

namespace CPN {
	class QueueInfo;
	class NodeInfo;

	class NodeQueueReader : public QueueReader {
	public:
		NodeQueueReader(NodeInfo* nodeinfo_, const std::string &portname_)
			: nodeinfo(nodeinfo_), portname(portname_) {}

		virtual void SetQueue(QueueInfo* queueinfo_) = 0;

		virtual QueueInfo* GetQueue(void) = 0;

		virtual PthreadCondition* GetEvent(void) = 0;

		NodeInfo* GetNodeInfo(void) { return nodeinfo; }

		const std::string &GetPortName(void) const { return portname; }
	private:
		NodeInfo* nodeinfo;
		const std::string portname;
	};
}

#endif
