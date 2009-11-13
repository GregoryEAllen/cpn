/** \file
 */

#ifndef RANDOMINSTRUCTIONNODE_H
#define RANDOMINSTRUCTIONNODE_H

#include "NodeBase.h"
#include "RandomInstructionGenerator.h"
#include "QueueWriter.h"
#include "QueueReader.h"

class Variant;

#define RANDOMINSTRUCTIONNODE_TYPENAME "RandomInstructionNodeType"

#define KERNEL_NAME_FORMAT "K #%u"

class RandomInstructionNode : public CPN::NodeBase, private RandomInstructionGenerator {
public:

    struct RINState {

        RINState(const Variant &args);

        RINState(unsigned id, unsigned iter, unsigned nk, RandomInstructionGenerator::State s)
            : nodeID(id), iterations(iter), numKernels(nk), state(s) {}

        std::string ToJSON();

        unsigned nodeID;
        unsigned iterations;
        unsigned numKernels;
        RandomInstructionGenerator::State state;
    };

    RandomInstructionNode(CPN::Kernel& ker, const CPN::NodeAttr& attr,
            const RINState &initialState);
    ~RandomInstructionNode() {}

    void Process();

    static void RegisterNodeType();

    static std::string GetNodeNameFromID(unsigned id);

    static void CreateRIN(CPN::Kernel& kernel, unsigned iterations,
        unsigned numNodes, unsigned debugLevel, LFSR::LFSR_t seed, unsigned numKernels);
private:
    /// Random Instruction Node IDentifier
    unsigned myID;
    unsigned iterations;
    bool die;
    unsigned numKernels;
    void DoCreateNode(unsigned newNodeID, unsigned creatorNodeID);
    void DoDeleteNode(unsigned nodeID);
    void DoProducerNode(unsigned nodeID, unsigned dstNodeID);
    void DoTransmuterNode(unsigned nodeID, unsigned srcNodeID, unsigned dstNodeID);
    void DoConsumerNode(unsigned nodeID, unsigned srcNodeID);
    int dbprintf(int dbLevel, const char *fmt, ...);
    void CreateQueue(unsigned srcID, unsigned dstID);
    std::string CurrentInPort();
    std::string CurrentOutPort();
};

#endif

