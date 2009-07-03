/** \file
 */

#ifndef CPN_NODEQUEUEREADER_H
#define CPN_NODEQUEUEREADER_H

#include "QueueReader.h"
#include <string>

namespace CPN {
	class QueueInfo;
	class NodeInfo;
	/**
	 * \brief The queue reader as the Kernel and NodeInfo see it.
	 *
	 * Contains getter and setter methods for blocking
	 * and keeping track of who is connected to who.
	 */
	class NodeQueueReader : public QueueReader {
	public:
		NodeQueueReader(NodeInfo* nodeinfo_, const std::string &portname_)
			: nodeinfo(nodeinfo_), portname(portname_) {}

		/**
		 * Set the queue that the reader should use to read with.
		 * \param queueinfo_ the QueueInfo object that holds the queue
		 */
		virtual void SetQueueInfo(QueueInfo* queueinfo_) = 0;

		/**
		 * \return the QueueInfo object registered with us or 0
		 */
		virtual QueueInfo* GetQueueInfo(void) = 0;

		/**
		 * Sets the reader to terminate. Next call to a reader
		 * function will cause the node to stop.
		 */
		virtual void Terminate(void) = 0;

		/**
		 * \return a pointer to the NodeInfo we are associated with
		 */
		NodeInfo* GetNodeInfo(void) { return nodeinfo; }

		/**
		 * \return our name
		 */
		const std::string &GetPortName(void) const { return portname; }
	private:
		NodeInfo* nodeinfo;
		const std::string portname;
	};
}

#endif
