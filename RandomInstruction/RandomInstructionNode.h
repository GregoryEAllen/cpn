/** \file
 */

#ifndef RANDOMINSTRUCTIONNODE_H
#define RANDOMINSTRUCTIONNODE_H

#include "NodeBase.h"
#include "RandomInstructionGenerator.h"
#include "QueueWriter.h"
#include "QueueReader.h"

#define RANDOMINSTRUCTIONNODE_TYPENAME "RandomInstructionNodeType"

struct RINState {
	RINState(unsigned id, unsigned iter, RandomInstructionGenerator::State s)
		: nodeID(id), iterations(iter), state(s) {}
	RINState(unsigned id, unsigned iter, int dbglvl, unsigned nnode)
		: nodeID(id), iterations(iter),
		state(nnode, dbglvl) {}
	unsigned nodeID;
	unsigned iterations;
	RandomInstructionGenerator::State state;
};

class RandomInstructionNode : public CPN::NodeBase, private RandomInstructionGenerator {
public:
	RandomInstructionNode(CPN::Kernel& ker, const CPN::NodeAttr& attr,
			RINState initialState);
	~RandomInstructionNode() {}

	void Process(void);

	static void RegisterNodeType(void);
	static std::string GetNodeNameFromID(unsigned id);
	static void CreateRIN(CPN::Kernel& kernel, unsigned iterations,
		unsigned numNodes, unsigned debugLevel);
private:
	/// Random Instruction Node IDentifier
	unsigned myID;
	unsigned iterations;
	bool die;
	void DoCreateNode(unsigned newNodeID, unsigned creatorNodeID);
	void DoDeleteNode(unsigned nodeID);
	void DoProducerNode(unsigned nodeID, unsigned dstNodeID);
	void DoTransmuterNode(unsigned nodeID, unsigned srcNodeID, unsigned dstNodeID);
	void DoConsumerNode(unsigned nodeID, unsigned srcNodeID);
	int dbprintf(int dbLevel, const char *fmt, ...);
	void CreateQueue(unsigned srcID, unsigned dstID);
	std::string GetQueueName(unsigned srcID, unsigned dstID);
	std::string CurrentInPort(void);
	std::string CurrentOutPort(void);
};

#endif

