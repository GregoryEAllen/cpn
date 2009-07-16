/** \file
 * \brief Declaration of the NodeInfo class.
 */
#ifndef CPN_NODEINFO_H
#define CPN_NODEINFO_H

#include "common.h"
#include "NodeBase.h"
#include <map>
#include <string>

namespace CPN {
	class NodeBase;
	class NodeQueueReader;
	class NodeQueueWriter;
	class NodeFactory;
	class Kernel;
	class NodeAttr;
	class QueueInfo;

	/**
	 * \brief This class holds the node reference and information
	 * about the node's connections.
	 *
	 * This class is only used by NodeQueueReader, NodeQueueWriter,
	 * QueueInfo, and the Kernel.
	 */
	class CPN_LOCAL NodeInfo {
	public:
		NodeInfo(Kernel &ker, const NodeAttr &attr,
			const void* const arg, const ulong argsize);

		~NodeInfo();

		/**
		 * Terminate the node
		 */
		void Terminate(void);

		/**
		 * Connect the given queue to the given portname.
		 * This function will call the necessary function
		 * in the queue info to setup the connection.
		 * \param queue the queueinfo object that holds the queue
		 * \param portname the name of the port to connect
		 */
		void SetWriter(QueueInfo* queue, const std::string &portname);

		/**
		 * Clear the writer associated with the portname.
		 * This function indicates to the writer that
		 * we are done with the queue in question.
		 * \param portname
		 */
		void ClearWriter(const std::string &portname);

		/**
		 * Get the NodeQueueWriter that corresponds to the
		 * given portname
		 * \param name the name of the port
		 * \return the NodeQueueWriter
		 */
		NodeQueueWriter* GetWriter(const std::string &name);

		/**
		 * Conct the given queeu to the given portname.
		 * \see CPN::NodeInfo::SetWriter
		 * \param queue the queue info that holds the queue
		 * \param portname the name of the port
		 */
		void SetReader(QueueInfo* queue, const std::string &portname);

		/**
		 * Clear the reader associated with the portname.
		 * This function indicates to the reader that
		 * we are done with the queue in question.
		 * \param portname
		 */
		void ClearReader(const std::string &portname);

		/**
		 * Get the NodeQueueReader that corresponds to the given
		 * portname.
		 * \see CPN::NodeInfo::GetWriter
		 * \param name the name of the port
		 * \return the NodeQueueReader
		 */
		NodeQueueReader* GetReader(const std::string &name);

		/**
		 * Get the node this object holds.
		 * \return the node
		 */
		NodeBase* GetNode(void) { return node; }

		const std::string GetName(void) const { return node->GetName(); }

	private:

		NodeFactory* factory;
		NodeBase* node;
		std::map<std::string, NodeQueueReader*> inputs;
		std::map<std::string, NodeQueueWriter*> outputs;
	};
}
#endif
