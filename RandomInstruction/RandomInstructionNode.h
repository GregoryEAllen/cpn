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
	RINState() : nodeID(0), iterations(100), debugLevel(0) {}
	unsigned nodeID;
	unsigned iterations;
	unsigned debugLevel;
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
	static void CreateNode(CPN::Kernel& kernel, RINState state);
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

	CPN::QueueWriter* rawout;
	CPN::QueueReader* rawin;
};

#endif

