/** \file
 */

#ifndef CPN_NODEQUEUEWRITER_H
#define CPN_NODEQUEUEWRITER_H

#include "QueueWriter.h"
#include <string>

class PthreadCondition;

namespace CPN {
	class QueueInfo;
	class NodeInfo;

	/**
	 * The queue writer as the Kernel and NodeInfo see it.
	 * Contains getter and setter methods for blocking
	 * and keeping track of who is connected to who.
	 */
	class NodeQueueWriter : public QueueWriter {
	public:
		NodeQueueWriter(NodeInfo* nodeinfo_, const std::string &portname_) 
			: nodeinfo(nodeinfo_), portname(portname_) {}

		virtual void SetQueueInfo(QueueInfo* queue_) = 0;

		virtual QueueInfo* GetQueueInfo(void) = 0;

		virtual PthreadCondition* GetEvent(void) = 0;

		virtual void Terminate(void) = 0;

		NodeInfo* GetNodeInfo(void) { return nodeinfo; }

		const std::string &GetPortName(void) const { return portname; }
	private:
		NodeInfo* nodeinfo;
		const std::string portname;
	};
}

#endif
