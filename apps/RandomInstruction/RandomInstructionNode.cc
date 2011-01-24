/** \file
 */

#include "RandomInstructionNode.h"
#include "NodeFactory.h"
#include "OQueue.h"
#include "IQueue.h"
#include "Kernel.h"
#include "Variant.h"
#include "ToString.h"
#include "VariantToJSON.h"
#include "JSONToVariant.h"
#include <stdio.h>
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
        JSONToVariant parse;
        parse.Parse(attr.GetParam().data(), attr.GetParam().size());
        ASSERT(parse.Done());
        Variant args = parse.Get();
        return CPN::shared_ptr<CPN::NodeBase>(
                new RandomInstructionNode(ker, attr, RandomInstructionNode::RINState(args))
                );
    }
};

RandomInstructionNode::RINState::RINState(const Variant &args) {
    nodeID = args["nodeID"].AsUnsigned();
    iterations = args["iterations"].AsUnsigned();
    Variant::List::const_iterator ki = args["kernelnames"].AsArray().begin();
    while (ki != args["kernelnames"].AsArray().end()) {
        kernelnames.push_back(ki->AsString());
        ++ki;
    }
    if (args["state"].IsObject()) {
        // We have state for the RIG
        Variant s = args["state"];
        state.feed = s["feed"].AsNumber<LFSR::LFSR_t>();
        state.seed = s["seed"].AsNumber<LFSR::LFSR_t>();
        state.maxID = s["maxID"].AsUnsigned();
        state.debugLevel = s["debugLevel"].AsInt();
        Variant::List::const_iterator i = s["liveNodes"].AsArray().begin();
        while (i != s["liveNodes"].AsArray().end()) {
            state.liveNodes.push_back(i->AsUnsigned());
            ++i;
        }
    } else {
        if (args["feed"].IsNumber()) {
            state.feed = args["feed"].AsNumber<LFSR::LFSR_t>();
        } else {
            state.feed = RandomInstructionGenerator::DEFAULT_FEED;
        }
        if (args["seed"].IsNumber()) {
            state.seed = args["seed"].AsNumber<LFSR::LFSR_t>();
        } else {
            state.seed = RandomInstructionGenerator::DEFAULT_SEED;
        }

        if (args["debugLevel"].IsNumber()) {
            state.debugLevel = args["debugLevel"].AsInt();
        } else {
            state.debugLevel = 0;
        }
        unsigned numNodes = args["numNodes"].AsUnsigned();
        state.maxID = numNodes;
        for (unsigned i = 0; i < numNodes; ++i) {
            state.liveNodes.push_back(i);
        }
    }
}

std::string RandomInstructionNode::RINState::ToJSON() {
    Variant rinstate(Variant::ObjectType);
    rinstate["nodeID"] = nodeID;
    rinstate["iterations"] = iterations;
    rinstate["kernelnames"] = Variant(Variant::ArrayType);
    std::vector<std::string>::iterator ki = kernelnames.begin();
    while (ki != kernelnames.end()) {
        rinstate["kernelnames"].Append(*ki);
        ++ki;
    }
    Variant rigstate(Variant::ObjectType);
    rigstate["feed"] = state.feed;
    rigstate["seed"] = state.seed;
    rigstate["debugLevel"] = state.debugLevel;
    rigstate["maxID"] = state.maxID;
    Variant liveNodes(Variant::ArrayType);
    std::deque<unsigned>::iterator i = state.liveNodes.begin();
    while (i != state.liveNodes.end()) {
        liveNodes.Append(Variant(*i));
        ++i;
    }
    rigstate["liveNodes"] = liveNodes;
    rinstate["state"] = rigstate;
    return VariantToJSON(rinstate);
}

RandomInstructionNode::RandomInstructionNode(CPN::Kernel& ker,
    const CPN::NodeAttr& attr, const RINState &initialState)
:   CPN::NodeBase(ker, attr), RandomInstructionGenerator(initialState.state) {
    myID = initialState.nodeID;
    iterations = initialState.iterations;
    kernelnames = initialState.kernelnames;
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
        unsigned numNodes, unsigned debugLevel, LFSR::LFSR_t seed, const std::vector<std::string> &kernelnames) {

    Variant state(Variant::ObjectType);
    state["iterations"] = iterations;
    state["seed"] = seed;
    state["debugLevel"] = debugLevel;
    state["numNodes"] = numNodes;
    std::vector<std::string>::const_iterator ki = kernelnames.begin();
    while (ki != kernelnames.end()) {
        state["kernelnames"].Append(*ki);
        ++ki;
    }
    for (unsigned i = 0; i < numNodes; ++i) {
        state["nodeID"] = i;
        CPN::NodeAttr attr(GetNodeNameFromID(i),
                RANDOMINSTRUCTIONNODE_TYPENAME);
        attr.SetParam(VariantToJSON(state));
        attr.SetHost(kernelnames[(i % kernelnames.size())]);
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
        RINState state = RINState(newNodeID, iterations, kernelnames, GetState());
        CPN::NodeAttr attr(GetNodeNameFromID(state.nodeID),
                RANDOMINSTRUCTIONNODE_TYPENAME);
        attr.SetParam(state.ToJSON());
        attr.SetHost(kernelnames[(newNodeID % kernelnames.size())]);
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

        CPN::OQueue<unsigned> out = GetWriter(CurrentOutPort());
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

        CPN::OQueue<unsigned> out = GetWriter(CurrentOutPort());
        CPN::IQueue<unsigned> in = GetReader(CurrentInPort());
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
        CPN::IQueue<unsigned> in = GetReader(CurrentInPort());
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
    qattr.SetDatatype<unsigned>();
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

extern "C" CPN::shared_ptr<CPN::NodeFactory> cpninitRandomInstructionNodeType(void);

CPN::shared_ptr<CPN::NodeFactory> cpninitRandomInstructionNodeType(void) {
    return (CPN::shared_ptr<CPN::NodeFactory>(new RINFactory));
}
