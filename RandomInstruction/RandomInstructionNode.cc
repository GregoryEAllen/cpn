/** \file
 */

#include "RandomInstructionNode.h"
#include "NodeFactory.h"
#include "QueueWriterAdapter.h"
#include "QueueReaderAdapter.h"
#include "Kernel.h"
#include "ToString.h"
#include <stdexcept>
#include <stdarg.h>

const char* const OUT_PORT_FORMAT = "Out:%lu";
const char* const IN_PORT_FORMAT = "In:%lu";
const char* const NODE_NAME_FORMAT = "RIN #%u";
const char* const QUEUE_NAME_FORMAT = "Q#%lu:%u->%u";

class RINFactory : public CPN::NodeFactory {
public:
    RINFactory() : CPN::NodeFactory(RANDOMINSTRUCTIONNODE_TYPENAME) {}
    ~RINFactory() {}
    CPN::shared_ptr<CPN::NodeBase> Create(CPN::Kernel& ker, const CPN::NodeAttr& attr) {
        RINState* rins = (RINState*)(attr.GetArg().GetBuffer());
        return CPN::shared_ptr<CPN::NodeBase>(new RandomInstructionNode(ker, attr, *rins));
    }
};

RandomInstructionNode::RandomInstructionNode(CPN::Kernel& ker,
    const CPN::NodeAttr& attr, RINState initialState)
:   CPN::NodeBase(ker, attr), RandomInstructionGenerator(initialState.state) {
    myID = initialState.nodeID;
    iterations = initialState.iterations;
    die = false;
}

void RandomInstructionNode::Process(void) {
    dbprintf(1,"Started\n");
    while (iterations-- > 0 && !die) {
        if (debugLevel < 0) {
            printf(".");
        }
        RunOnce();
    }
    dbprintf(1,"Stopped\n");
}

void RandomInstructionNode::CreateRIN(CPN::Kernel& kernel, unsigned iterations,
        unsigned numNodes, unsigned debugLevel, LFSR::LFSR_t seed) {
    for (unsigned i = 0; i < numNodes; ++i) {
        RINState state = RINState(i, iterations, debugLevel, numNodes);
        state.state.seed = seed;
        CPN::NodeAttr attr(GetNodeNameFromID(state.nodeID),
                RANDOMINSTRUCTIONNODE_TYPENAME);
        attr.SetParam(StaticConstBuffer(&state, sizeof(state)));
        kernel.CreateNode(attr);
    }
}

/*
 * Note that all nodes must be in sync when creating or deleting a node.  If
 * they are not we will end up with deadlock. Or the situation where we are
 * trying to communicate with a dead node or a node is scheduled to die then be
 * created again and we are tyring to communicate to the node in its recreated
 * state before it dies.
 */
void RandomInstructionNode::DoCreateNode(unsigned newNodeID, unsigned creatorNodeID) {
    if (creatorNodeID == myID) {
        RandomInstructionGenerator::DoCreateNode(newNodeID, creatorNodeID);
        RINState state = RINState(newNodeID, iterations, GetState());
        CPN::NodeAttr attr(GetNodeNameFromID(state.nodeID),
                RANDOMINSTRUCTIONNODE_TYPENAME);
        attr.SetParam(StaticConstBuffer(&state, sizeof(state)));
        kernel.CreateNode(attr);
    }
    if (newNodeID == myID) {
        ASSERT(false, "A node is undead!");
    }
    kernel.WaitNodeStart(GetNodeNameFromID(newNodeID));
}

void RandomInstructionNode::DoDeleteNode(unsigned nodeID) {
    if (myID == nodeID) {
        ASSERT(!die);
        RandomInstructionGenerator::DoDeleteNode(nodeID);
        die = true;
    } else {
        kernel.WaitNodeTerminate(GetNodeNameFromID(nodeID));
    }
}

void RandomInstructionNode::DoProducerNode(unsigned nodeID, unsigned dstNodeID) {
    if (myID == nodeID) {
        ASSERT(!die);
        //RandomInstructionGenerator::DoProducerNode(nodeID, dstNodeID);

        CreateQueue(nodeID, dstNodeID);

        CPN::QueueWriterAdapter<unsigned> out = GetWriter(CurrentOutPort());
        unsigned *data = out.GetEnqueuePtr(2);
        dbprintf(1, "(%u) =%lu> %u\n", nodeID, out.GetKey(), dstNodeID);
        data[0] = nodeID;
        data[1] = dstNodeID;
        out.Enqueue(2);
        out.Release();
    }
}

void RandomInstructionNode::DoTransmuterNode(unsigned nodeID, unsigned srcNodeID,
            unsigned dstNodeID) {
    if (myID == nodeID) {
        ASSERT(!die);
        //RandomInstructionGenerator::DoTransmuterNode(nodeID, srcNodeID, dstNodeID);

        CreateQueue(nodeID, dstNodeID);

        CPN::QueueWriterAdapter<unsigned> out = GetWriter(CurrentOutPort());
        CPN::QueueReaderAdapter<unsigned> in = GetReader(CurrentInPort());
        const unsigned *indata = in.GetDequeuePtr(2);
        dbprintf(1, "%u =%lu> (%u) -> %u\n", srcNodeID, in.GetKey(), nodeID, dstNodeID);
        ASSERT(indata);
        if (indata[0] != srcNodeID) {
            dbprintf(1, "!!! Got %u was expecting %u\n", indata[0], srcNodeID);
        }
        if (indata[1] != nodeID) {
            dbprintf(1, "!!! Got %u was expecting %u\n", indata[1], nodeID);
        }
        in.Dequeue(2);
        in.Release();
        unsigned *outdata = out.GetEnqueuePtr(2);
        dbprintf(1, "%u -> (%u) =%lu> %u\n", srcNodeID, nodeID, out.GetKey(), dstNodeID);
        outdata[0] = nodeID;
        outdata[1] = dstNodeID;
        out.Enqueue(2);
        out.Release();
    }
}

void RandomInstructionNode::DoConsumerNode(unsigned nodeID, unsigned srcNodeID) {
    if (myID == nodeID) {
        ASSERT(!die);
        //RandomInstructionGenerator::DoConsumerNode(nodeID, srcNodeID);
        CPN::QueueReaderAdapter<unsigned> in = GetReader(CurrentInPort());
        const unsigned *indata = in.GetDequeuePtr(2);
        dbprintf(1, "%u =%lu> (%u)\n", srcNodeID, in.GetKey(), nodeID);
        ASSERT(indata);
        if (indata[0] != srcNodeID) {
            dbprintf(1, "!!! Got %u was expecting %u\n", indata[0], srcNodeID);
        }
        if (indata[1] != nodeID) {
            dbprintf(1, "!!! Got %u was expecting %u\n", indata[1], nodeID);
        }
        in.Dequeue(2);
        in.Release();
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
    CPN::QueueAttr qattr(1024, 1024);
    qattr.SetWriter(GetNodeNameFromID(srcID), CurrentOutPort());
    qattr.SetReader(GetNodeNameFromID(dstID), CurrentInPort());
    kernel.CreateQueue(qattr);
}

std::string RandomInstructionNode::GetNodeNameFromID(unsigned id) {
    return ToString(NODE_NAME_FORMAT, id);
}

std::string RandomInstructionNode::CurrentInPort() {
    return ToString(IN_PORT_FORMAT, lfsr.Seed());
}

std::string RandomInstructionNode::CurrentOutPort() {
    return ToString(OUT_PORT_FORMAT, lfsr.Seed());
}

void RandomInstructionNode::RegisterNodeType() {
    CPNRegisterNodeFactory(CPN::shared_ptr<CPN::NodeFactory>(new RINFactory));
}
