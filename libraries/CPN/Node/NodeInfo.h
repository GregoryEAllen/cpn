/** \file
 * \brief Declaration of the NodeInfo class.
 */
#ifndef CPN_NODEINFO_H
#define CPN_NODEINFO_H

#include "NodeFactory.h"
#include <map>
#include <string>

namespace CPN {
	class NodeBase;
	class QueueReader;
	class QueueWriter;
	/**
	 * This class holds the node reference and information
	 * about the node's connections.
	 */
	class NodeInfo {
	public:
		NodeInfo(Kernel &ker, NodeFactory* factory_, const NodeAttr attr,
			const void* const arg, const ulong argsize)
	       : factory(factory_)
		{
			node = factory->Create(ker, attr, arg, argsize);
		}

		~NodeInfo() {
			factory->Destroy(node);
			node = 0;
		}

		void SetWriter(std::string name, QueueWriter* qwriter) {
			outputs[name] = qwriter;
		}

		QueueWriter* GetWriter(std::string name) {
			return outputs[name];
		}

		void SetReader(std::string name, QueueReader* qreader) {
			inputs[name] = qreader;
		}

		QueueReader* GetReader(std::string name) {
			return inputs[name];
		}

		NodeBase* GetNode(void) { return node; }

	private:
		NodeFactory* factory;
		NodeBase* node;
		std::map<std::string, QueueReader*> inputs;
		std::map<std::string, QueueWriter*> outputs;
	};
}
#endif
