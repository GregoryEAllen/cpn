/** \file
 */

#ifndef RANDOMINSTRUCTIONNODE_H
#define RANDOMINSTRUCTIONNODE_H

#include "NodeBase.h"
#include "RandomInstructionGenerator.h"
#include "QueueWriter.h"
#include "QueueReader.h"
#include "Barrier.h"

#define RANDOMINSTRUCTIONNODE_TYPENAME "RandomInstructionNodeType"

struct RINState {
    RINState(unsigned id, unsigned iter, CPN::shared_ptr<Sync::Barrier> b, RandomInstructionGenerator::State s)
        : nodeID(id), iterations(iter), state(s), barrier(b) {}
    RINState(unsigned id, unsigned iter, int dbglvl, unsigned nnode, CPN::shared_ptr<Sync::Barrier> b)
        : nodeID(id), iterations(iter),
        state(nnode, dbglvl), barrier(b) {}
    unsigned nodeID;
    unsigned iterations;
    RandomInstructionGenerator::State state;
    CPN::shared_ptr<Sync::Barrier> barrier;
};

class RandomInstructionNode : public CPN::NodeBase, private RandomInstructionGenerator {
public:
    RandomInstructionNode(CPN::Kernel& ker, const CPN::NodeAttr& attr,
            RINState initialState);
    ~RandomInstructionNode() {}

    void Process();

    static void RegisterNodeType();
    static std::string GetNodeNameFromID(unsigned id);
    static void CreateRIN(CPN::Kernel& kernel, unsigned iterations,
        unsigned numNodes, unsigned debugLevel, LFSR::LFSR_t seed);
private:
    /// Random Instruction Node IDentifier
    unsigned myID;
    unsigned iterations;
    bool die;
    CPN::shared_ptr<Sync::Barrier> barrier;
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

