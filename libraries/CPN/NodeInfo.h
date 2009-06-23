/** \file
 * \brief Declaration of the NodeInfo class.
 */
#ifndef CPN_NODEINFO_H
#define CPN_NODEINFO_H

#include <map>
#include <string>

namespace CPN {
	class NodeBase;
	class NodeQueueReader;
	class NodeQueueWriter;
	class NodeFactory;
	class Kernel;
	class NodeAttr;

	/**
	 * This class holds the node reference and information
	 * about the node's connections.
	 */
	class NodeInfo {
	public:
		NodeInfo(Kernel &ker, const NodeAttr attr,
			const void* const arg, const ulong argsize);

		~NodeInfo();

		void SetWriter(QueueInfo* queue, std::string portname);

		NodeQueueWriter* GetWriter(std::string name);

		void SetReader(QueueInfo* queue, std::string portname);

		NodeQueueReader* GetReader(std::string name);

		NodeBase* GetNode(void) { return node; }

	private:

		NodeFactory* factory;
		NodeBase* node;
		std::map<std::string, NodeQueueReader*> inputs;
		std::map<std::string, NodeQueueWriter*> outputs;
	};
}
#endif
