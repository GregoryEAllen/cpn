/** \file
 */

#ifndef CPN_NODEQUEUEWRITER_H
#define CPN_NODEQUEUEWRITER_H

#include "QueueWriter.h"
#include "NodeQueueEndpoint.h"

namespace CPN {
	class QueueInfo;
	class NodeInfo;

	/**
	 * \brief The queue writer as the Kernel and NodeInfo see it.
	 *
	 * Contains getter and setter methods for blocking
	 * and keeping track of who is connected to who.
	 */
	class NodeQueueWriter : public QueueWriter, public NodeQueueEndpoint {
	public:

		NodeQueueWriter(NodeInfo* nodeinfo_, const std::string &portname_) 
			: NodeQueueEndpoint(nodeinfo_, portname_) {}

		virtual ~NodeQueueWriter() {}
	protected:
		void SetQueueInfoEndpoint(QueueInfo* qinfo) {
			if (qinfo) { qinfo->SetWriter(this); }
		}
		void ClearQueueInfoEndpoint(QueueInfo* qinfo, bool checkdeath) {
			if (qinfo) qinfo->ClearWriter(checkdeath);
		}
	};
}

#endif
