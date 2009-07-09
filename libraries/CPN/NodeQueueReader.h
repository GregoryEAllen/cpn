/** \file
 */

#ifndef CPN_NODEQUEUEREADER_H
#define CPN_NODEQUEUEREADER_H

#include "QueueReader.h"
#include "NodeQueueEndpoint.h"

namespace CPN {

	class NodeInfo;

	/**
	 * \brief The queue reader as the Kernel and NodeInfo see it.
	 *
	 * Contains getter and setter methods for blocking
	 * and keeping track of who is connected to who.
	 */
	class NodeQueueReader : public QueueReader, public NodeQueueEndpoint {
	public:

		NodeQueueReader(NodeInfo* nodeinfo_, const std::string &portname_)
			: NodeQueueEndpoint(nodeinfo_, portname_) {}

		virtual ~NodeQueueReader() {}
	protected:
		void SetQueueInfoEndpoint(void) {
			if (queueinfo) { queueinfo->SetReader(this); }
		}
		void ClearQueueInfoEndpoint(void) {
			if (queueinfo) { queueinfo->ClearReader(); }
		}
	private:
	};
}

#endif
