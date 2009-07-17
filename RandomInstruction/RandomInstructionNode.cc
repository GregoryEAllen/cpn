/** \file
 */

#include "RandomInstructionNode.h"
#include "NodeFactory.h"
#include "QueueWriterAdapter.h"
#include "QueueReaderAdapter.h"
#include "Kernel.h"
#include "ToString.h"
#include <stdexcept>
#include <cassert>
#include <stdarg.h>

const char* const OUT_PORT_FORMAT = "Out:%lu";
const char* const IN_PORT_FORMAT = "In:%lu";
const char* const NODE_NAME_FORMAT = "RIN #%u";
const char* const QUEUE_NAME_FORMAT = "Q#%lu:%u->%u";

class RINFactory : public CPN::NodeFactory {
public:
	RINFactory() : CPN::NodeFactory(RANDOMINSTRUCTIONNODE_TYPENAME) {}
	~RINFactory() {}
	CPN::NodeBase* Create(CPN::Kernel& ker, const CPN::NodeAttr& attr,
			const void* const arg, const unsigned long argsize) {
		RINState* rins = (RINState*)(arg);
		return new RandomInstructionNode(ker, attr, *rins);
	}
	CPN::NodeBase* Create(CPN::Kernel& ker, const CPN::NodeAttr& attr) {
		throw std::invalid_argument("Must pass a RINState object");
	}
	void Destroy(CPN::NodeBase* node) {
		delete node;
	}
};
static RINFactory factoryInstance;

RandomInstructionNode::RandomInstructionNode(CPN::Kernel& ker,
	const CPN::NodeAttr& attr, RINState initialState)
:	CPN::NodeBase(ker, attr), RandomInstructionGenerator(initialState.state) {
	myID = initialState.nodeID;
	iterations = initialState.iterations;
	die = false;
}

void RandomInstructionNode::Process(void) {
	dbprintf(1,"Started\n");
	while (iterations-- > 0) {
		RunOnce();
	}
	dbprintf(1,"Stopped\n");
}

void RandomInstructionNode::CreateRIN(CPN::Kernel& kernel, unsigned iterations,
		unsigned numNodes, unsigned debugLevel) {
	for (unsigned i = 0; i < numNodes; ++i) {
		RINState state = RINState(i, iterations, debugLevel, numNodes);
		kernel.CreateNode(GetNodeNameFromID(state.nodeID),
			RANDOMINSTRUCTIONNODE_TYPENAME, &state, sizeof(RINState));
	}
}

void RandomInstructionNode::DoCreateNode(unsigned newNodeID, unsigned creatorNodeID) {
	if (creatorNodeID == myID) {
		RandomInstructionGenerator::DoCreateNode(newNodeID, creatorNodeID);
		RINState state = RINState(newNodeID, iterations, GetState());
		try {
			kernel.CreateNode(GetNodeNameFromID(state.nodeID),
				RANDOMINSTRUCTIONNODE_TYPENAME, &state, 0);
		} catch (std::invalid_argument e) {
			// It's already there
			// maybe this should be changed to a return value instead?
		}
	}
	if (newNodeID == myID) {
		assert(die);
		die = false;
	}
}

void RandomInstructionNode::DoDeleteNode(unsigned nodeID) {
	if (myID == nodeID) {
		assert(!die);
		RandomInstructionGenerator::DoDeleteNode(nodeID);
		die = true;
	}
}

void RandomInstructionNode::DoProducerNode(unsigned nodeID, unsigned dstNodeID) {
	if (myID == nodeID) {
		assert(!die);
		RandomInstructionGenerator::DoProducerNode(nodeID, dstNodeID);

		CreateQueue(nodeID, dstNodeID);
		kernel.ConnectWriteEndpoint(GetQueueName(nodeID, dstNodeID),
				GetName(), CurrentOutPort());
		kernel.ConnectReadEndpoint(GetQueueName(nodeID, dstNodeID),
				GetNodeNameFromID(dstNodeID), CurrentInPort());

		CPN::QueueWriterAdapter<unsigned> out = kernel.GetWriter(GetName(),
			       	CurrentOutPort());
		unsigned *data = out.GetEnqueuePtr(2);
		data[0] = nodeID;
		data[1] = dstNodeID;
		out.Enqueue(2);
		kernel.ReleaseWriter(GetName(), CurrentOutPort());
	}
}

void RandomInstructionNode::DoTransmuterNode(unsigned nodeID, unsigned srcNodeID,
	       	unsigned dstNodeID) {
	if (myID == nodeID) {
		assert(!die);
		RandomInstructionGenerator::DoTransmuterNode(nodeID, srcNodeID, dstNodeID);

		CreateQueue(nodeID, dstNodeID);
		kernel.ConnectWriteEndpoint(GetQueueName(nodeID, dstNodeID),
				GetName(), CurrentOutPort());
		kernel.ConnectReadEndpoint(GetQueueName(nodeID, dstNodeID),
				GetNodeNameFromID(dstNodeID), CurrentInPort());

		CPN::QueueWriterAdapter<unsigned> out = kernel.GetWriter(GetName(),
			       	CurrentOutPort());
		CPN::QueueReaderAdapter<unsigned> in = kernel.GetReader(GetName(),
				CurrentInPort());
		const unsigned *indata = in.GetDequeuePtr(2);
		if (indata[0] != srcNodeID) {
			dbprintf(1, "!!! Got %u was expecting %u\n", indata[0], srcNodeID);
		}
		if (indata[1] != nodeID) {
			dbprintf(1, "!!! Got %u was expecting %u\n", indata[1], nodeID);
		}
		in.Dequeue(2);
		unsigned *outdata = out.GetEnqueuePtr(2);
		outdata[0] = nodeID;
		outdata[1] = dstNodeID;
		out.Enqueue(2);
		kernel.ReleaseWriter(GetName(), CurrentOutPort());
		kernel.ReleaseReader(GetName(), CurrentInPort());
	}
}

void RandomInstructionNode::DoConsumerNode(unsigned nodeID, unsigned srcNodeID) {
	if (myID == nodeID) {
		assert(!die);
		RandomInstructionGenerator::DoConsumerNode(nodeID, srcNodeID);
		CPN::QueueReaderAdapter<unsigned> in = kernel.GetReader(GetName(),
				CurrentInPort());
		const unsigned *indata = in.GetDequeuePtr(2);
		if (indata[0] != srcNodeID) {
			dbprintf(1, "!!! Got %u was expecting %u\n", indata[0], srcNodeID);
		}
		if (indata[1] != nodeID) {
			dbprintf(1, "!!! Got %u was expecting %u\n", indata[1], nodeID);
		}
		in.Dequeue(2);
		kernel.ReleaseReader(GetName(), CurrentInPort());
	}
}

int RandomInstructionNode::dbprintf(int dbLevel, const char *fmt, ...) {
	int result = 0;
	if (dbLevel<=debugLevel) {
		std::string format = ToString("#%u: %s", myID, fmt);
		va_list args;
		va_start( args, fmt );
		result = vprintf(format.c_str(), args);
		va_end( args );
	}
	return result;
}

void RandomInstructionNode::CreateQueue(unsigned srcID, unsigned dstID) {
	kernel.CreateQueue(GetQueueName(srcID, dstID),
		CPN_QUEUETYPE_SIMPLE, 1024, 1024, 1);
}
std::string RandomInstructionNode::GetQueueName(unsigned srcID, unsigned dstID) {
	return ToString(QUEUE_NAME_FORMAT, lfsr.Seed(), srcID, dstID);
}

std::string RandomInstructionNode::GetNodeNameFromID(unsigned id) {
	return ToString(NODE_NAME_FORMAT, id);
}

std::string RandomInstructionNode::CurrentInPort(void) {
	return ToString(IN_PORT_FORMAT, lfsr.Seed());
}

std::string RandomInstructionNode::CurrentOutPort(void) {
	return ToString(OUT_PORT_FORMAT, lfsr.Seed());
}

void RandomInstructionNode::RegisterNodeType(void) {
	CPNRegisterNodeFactory(&factoryInstance);
}
