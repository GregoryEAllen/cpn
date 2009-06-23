/** \file
 * \brief Declaration of the NodeInfo class.
 */
#ifndef CPN_NODEINFO_H
#define CPN_NODEINFO_H

#include "NodeFactory.h"
#include "BlockingQueueWriter.h"
#include "BlockingQueueReader.h"
#include <map>
#include <string>
#include <stdexcept>
#include <algorithm>

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
		NodeInfo(Kernel &ker, const NodeAttr attr,
			const void* const arg, const ulong argsize)
		{
			factory = NodeFactory::GetFactory(attr.GetTypeName());
			if (!factory) throw ::std::invalid_argument("Node type name must be a valid registered type.");
			node = factory->Create(ker, attr, arg, argsize);
		}

		~NodeInfo() {
			factory->Destroy(node);
			node = 0;
			for_each(inputs.begin(), inputs.end(), DeleteReader);
			inputs.clear();
			for_each(outputs.begin(), outputs.end(), DeleteWriter);
			outputs.clear();
		}

		QueueWriter* GetWriter(std::string name) {
			if (!outputs[name]) {
				outputs[name] = new BlockingQueueWriter();
			}
			return outputs[name];
		}

		QueueReader* GetReader(std::string name) {
			if (!inputs[name]) {
				inputs[name] = new BlockingQueueReader();
			}
			return inputs[name];
		}

		NodeBase* GetNode(void) { return node; }

	private:
		static void DeleteReader(std::pair<std::string, QueueReader*> qr) {
			delete qr.second;
		}

		static void DeleteWriter(std::pair<std::string, QueueWriter*> qw) {
			delete qw.second;
		}

		NodeFactory* factory;
		NodeBase* node;
		std::map<std::string, QueueReader*> inputs;
		std::map<std::string, QueueWriter*> outputs;
	};
}
#endif
