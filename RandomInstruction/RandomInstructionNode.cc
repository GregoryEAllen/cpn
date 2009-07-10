/** \file
 */

#include "RandomInstructionNode.h"
#include "NodeFactory.h"
#include "QueueWriterAdapter.h"
#include "QueueReaderAdapter.h"
#include "Kernel.h"
#include "ToString.h"
#include <cassert>
#include <stdarg.h>

const char* const OUT_PORT = "y";
const char* const IN_PORT = "x";
const char* const NODE_NAME_FORMAT = "RIN #%u";

class RINFactory : public CPN::NodeFactory {
public:
	RINFactory() : CPN::NodeFactory(RANDOMINSTRUCTIONNODE_TYPENAME) {}
	~RINFactory() {}
	CPN::NodeBase* Create(CPN::Kernel& ker, const CPN::NodeAttr& attr,
			const void* const arg, const unsigned long argsize) {
		RINState* rins = (RINState*)(arg);
		return new RandomInstructionNode(ker, attr, *rins);
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
	debugLevel = initialState.debugLevel;
	die = false;
}

void RandomInstructionNode::Process(void) {
	dbprintf(1,"Started\n");
	rawout = kernel.GetWriter(GetName(), OUT_PORT);
	rawin = kernel.GetReader(GetName(), IN_PORT);
	while (iterations-- > 0 && !die) {
		RunOnce();
	}
	dbprintf(1,"Stopped\n");
}

void RandomInstructionNode::CreateNode(CPN::Kernel& kernel, RINState state) {
	kernel.CreateNode(GetNodeNameFromID(state.nodeID),
			RANDOMINSTRUCTIONNODE_TYPENAME, &state, 0);
	kernel.CreateQueue(GetNodeNameFromID(state.nodeID),
			CPN_QUEUETYPE_THRESHOLD, 1024, 1024, 1);
	kernel.ConnectReadEndpoint(GetNodeNameFromID(state.nodeID),
			GetNodeNameFromID(state.nodeID), IN_PORT);
}

void RandomInstructionNode::DoCreateNode(unsigned newNodeID, unsigned creatorNodeID) {
	if (creatorNodeID == myID) {
		RandomInstructionGenerator::DoCreateNode(newNodeID, creatorNodeID);
		RINState state;
		state.state = GetState();
		state.iterations = iterations;
		state.nodeID = newNodeID;
		state.debugLevel = debugLevel;
		CreateNode(kernel, state);
	}
}

void RandomInstructionNode::DoDeleteNode(unsigned nodeID) {
	if (myID == nodeID) {
		RandomInstructionGenerator::DoDeleteNode(nodeID);
		die = true;
	}
}

void RandomInstructionNode::DoProducerNode(unsigned nodeID, unsigned dstNodeID) {
	if (myID == nodeID) {
		RandomInstructionGenerator::DoProducerNode(nodeID, dstNodeID);
		kernel.ConnectWriteEndpoint(GetNodeNameFromID(dstNodeID),
				GetName(), OUT_PORT);
		CPN::QueueWriterAdapter<unsigned> out = rawout;
		unsigned *data = out.GetEnqueuePtr(2);
		data[0] = nodeID;
		data[1] = dstNodeID;
		out.Enqueue(2);
	}
}

void RandomInstructionNode::DoTransmuterNode(unsigned nodeID, unsigned srcNodeID,
	       	unsigned dstNodeID) {
	if (myID == nodeID) {
		RandomInstructionGenerator::DoTransmuterNode(nodeID, srcNodeID, dstNodeID);
		kernel.ConnectWriteEndpoint(GetNodeNameFromID(dstNodeID),
				GetName(), OUT_PORT);
		CPN::QueueWriterAdapter<unsigned> out = rawout;
		CPN::QueueReaderAdapter<unsigned> in = rawin;
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
	}
}

void RandomInstructionNode::DoConsumerNode(unsigned nodeID, unsigned srcNodeID) {
	if (myID == nodeID) {
		RandomInstructionGenerator::DoConsumerNode(nodeID, srcNodeID);
		CPN::QueueReaderAdapter<unsigned> in = rawin;
		const unsigned *indata = in.GetDequeuePtr(2);
		if (indata[0] != srcNodeID) {
			dbprintf(1, "!!! Got %u was expecting %u\n", indata[0], srcNodeID);
		}
		if (indata[1] != nodeID) {
			dbprintf(1, "!!! Got %u was expecting %u\n", indata[1], nodeID);
		}
		in.Dequeue(2);
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

std::string RandomInstructionNode::GetNodeNameFromID(unsigned id) {
	return ToString(NODE_NAME_FORMAT, id);
}

void RandomInstructionNode::RegisterNodeType(void) {
	CPNRegisterNodeFactory(&factoryInstance);
}
