//=============================================================================
//	$Id:$
//-----------------------------------------------------------------------------
//	RandomInstructionGenerator class for PN random case study
//-----------------------------------------------------------------------------
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================


#include "RandomInstructionGenerator.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <algorithm>
#include <sstream>


const RandomInstructionGenerator::LFSR_t RandomInstructionGenerator::DEFAULT_SEED = 1;
const RandomInstructionGenerator::LFSR_t RandomInstructionGenerator::DEFAULT_FEED = 0xF82F;

//-----------------------------------------------------------------------------
RandomInstructionGenerator::RandomInstructionGenerator()
//-----------------------------------------------------------------------------
:	lfsr(DEFAULT_FEED,DEFAULT_SEED)
{
	Initialize(2, 0.01, 0.01, 100);
}

//-----------------------------------------------------------------------------
RandomInstructionGenerator::RandomInstructionGenerator(const State& state)
//-----------------------------------------------------------------------------
:	lfsr(state.feed, state.seed)
{
	Initialize(state.debugLevel, 0.01, 0.01, state.numNodes);
	deletedNodes = state.deletedNodes;
}

void RandomInstructionGenerator::Initialize(int dbglvl, float probToCreateNode,
		float probToDeleteNode, unsigned startNumNodes)
{
	debugLevel = dbglvl;
	probabilityToCreateNode = probToCreateNode;
	probabilityToDeleteNode = probToDeleteNode;
	dbprintf(2,"# lfsr of order %d, with range 1-%d\n", lfsr.Order(), lfsr.MaxVal());
	ComputeRanges(startNumNodes);
}

//-----------------------------------------------------------------------------
int RandomInstructionGenerator::dbprintf(int dbLevel, const char *fmt, ...)
//-----------------------------------------------------------------------------
{
	int result = 0;
	va_list args;
	va_start( args, fmt );
	if (dbLevel<=debugLevel)
		result = vprintf(fmt, args);
	va_end( args );
	return result;
}


//-----------------------------------------------------------------------------
void RandomInstructionGenerator::ComputeRanges(unsigned nmNodes)
//-----------------------------------------------------------------------------
{
	numNodes = nmNodes;
	LFSR_t maxVal = lfsr.MaxVal();
	
	createRange = unsigned( floor(maxVal*probabilityToCreateNode+0.5) );
	deleteRange = unsigned( floor(maxVal*probabilityToDeleteNode+0.5) );

	dbprintf(2,"# numNodes = %d\n", numNodes);
	dbprintf(2,"# createRange = %d, deleteRange = %d\n", createRange, deleteRange);

	chainRange = ((maxVal-createRange-deleteRange)/numNodes) * numNodes; // int math
	noopRange = maxVal-createRange-deleteRange-chainRange;

	dbprintf(2,"# chainRange = %d, noopRange = %d\n", chainRange, noopRange);
	
//	unsigned totalRange = createRange + deleteRange + chainRange + noopRange;
//	dbprintf(3,"# totalRange = %d\n", totalRange);

	if (chainRange==0) {
		dbprintf(0,"Error: lfsr seed order is too small");
		exit(-1);
	}
}


//-----------------------------------------------------------------------------
void RandomInstructionGenerator::EndCurrentChain(void)
//-----------------------------------------------------------------------------
{
	if (currentChain.size()<=1) {
		dbprintf(2,"# discarding a chain of length %d\n", currentChain.size());
		currentChain.clear();
	} else {
		int idx;
		if (debugLevel >= 1) {
			std::ostringstream oss;
			//dbprintf(1,"create chain: [");
			oss << "create chain: [";
			for (idx=0; idx<currentChain.size()-1; idx++) {
				//dbprintf(1,"%lu, ", currentChain[idx]);
				oss << currentChain[idx] << ", ";
			}
			//dbprintf(1,"%lu]\n", currentChain[idx]);
			dbprintf(1, "%s%lu]\n", oss.str().c_str(), currentChain[idx]);
		}
		// create the chain
		DoProducerNode(currentChain[0], currentChain[1]);
		for (idx=1; idx<currentChain.size()-1; idx++) {
			DoTransmuterNode(currentChain[idx], currentChain[idx-1], currentChain[idx+1]);
		}
		DoConsumerNode(currentChain[idx], currentChain[idx-1]);
		currentChain.clear();
	}
}


//-----------------------------------------------------------------------------
void RandomInstructionGenerator::HandleChainOp(unsigned nodeID)
//-----------------------------------------------------------------------------
{
	if ( find( currentChain.begin(), currentChain.end(), nodeID ) != currentChain.end()  ) {
		// already in the current chain, end the chain
		EndCurrentChain();
	} else if ( find( deletedNodes.begin(), deletedNodes.end(), nodeID ) != deletedNodes.end()  ) {
		// can't add a deleted node, end the chain
		dbprintf(2,"## not chaining deleted node %lu\n", nodeID);
//		EndCurrentChain()
	} else {
		currentChain.push_back(nodeID);
	}
}


//-----------------------------------------------------------------------------
void RandomInstructionGenerator::HandleCreateOp(void)
//-----------------------------------------------------------------------------
{
	EndCurrentChain();

	// the lowest-numbered non-deleted node could create right away
	unsigned creatorNodeID = 0;
	// this could probably be much faster by sorting first
	while ( find( deletedNodes.begin(), deletedNodes.end(), creatorNodeID ) != deletedNodes.end() ) {
		creatorNodeID += 1;
	}
	
	unsigned newNodeID = 0;
	
	if (deletedNodes.size()>0) {
		newNodeID = deletedNodes[0];
		deletedNodes.pop_front();
	} else {
		newNodeID = numNodes;
		ComputeRanges(numNodes+1);
		dbprintf(2,"## numNodes is now %d\n", numNodes);
	}
	dbprintf(2,"## len(deletedNodes) %d\n", deletedNodes.size());
	DoCreateNode(newNodeID, creatorNodeID);
}


//-----------------------------------------------------------------------------
void RandomInstructionGenerator::HandleDeleteOp(unsigned nodeID)
//-----------------------------------------------------------------------------
{
	EndCurrentChain();

	if ( find( deletedNodes.begin(), deletedNodes.end(), nodeID ) != deletedNodes.end() ) {
		dbprintf(2,"## nodeID %d is already deleted!\n", nodeID);
		return;
	}
	// don't delete the last node
	if ( deletedNodes.size()==numNodes-1 ) {
		dbprintf(2,"## refusing to delete final node, %d\n", nodeID);
		return;
	}
	DoDeleteNode(nodeID);
	deletedNodes.push_back(nodeID);
	dbprintf(2,"## len(deletedNodes) %d\n", deletedNodes.size());

	// check to see if we should reduce numNodes
	unsigned newNumNodes = numNodes;
	while ( find( deletedNodes.begin(), deletedNodes.end(), newNumNodes-1 ) != deletedNodes.end() ) {
		newNumNodes -= 1;
	}
	if (newNumNodes>=numNodes) {
		return;
	}
	dbprintf(2,"## newNumNodes %d, numNodes %d\n", newNumNodes, numNodes);
	// reduce the number of nodes

	// remove from deletedNodes
	for (unsigned node=newNumNodes; node<numNodes; node++) {
		std::deque<unsigned>::iterator idx = find(deletedNodes.begin(), deletedNodes.end(), node);
		deletedNodes.erase(idx);
	}
	// and recompute
	ComputeRanges(newNumNodes);
}


//-----------------------------------------------------------------------------
void RandomInstructionGenerator::DoCreateNode(unsigned newNodeID, unsigned creatorNodeID)
//-----------------------------------------------------------------------------
{	
	dbprintf(1,"DoCreateNode %d, creatorNodeID %d\n", newNodeID, creatorNodeID);
}

//-----------------------------------------------------------------------------
void RandomInstructionGenerator::DoDeleteNode(unsigned nodeID)
//-----------------------------------------------------------------------------
{
	dbprintf(1,"DoDeleteNode %d\n", nodeID);
}

//-----------------------------------------------------------------------------
void RandomInstructionGenerator::DoProducerNode(unsigned nodeID, unsigned dstNodeID)
//-----------------------------------------------------------------------------
{
	dbprintf(1,"DoProducerNode (%d) -> %d\n", nodeID, dstNodeID);
}

//-----------------------------------------------------------------------------
void RandomInstructionGenerator::DoTransmuterNode(unsigned nodeID, unsigned srcNodeID, unsigned dstNodeID)
//-----------------------------------------------------------------------------
{
	dbprintf(1,"DoTransmuterNode %d -> (%d) -> %d\n", srcNodeID, nodeID, dstNodeID);
}

//-----------------------------------------------------------------------------
void RandomInstructionGenerator::DoConsumerNode(unsigned nodeID, unsigned srcNodeID)
//-----------------------------------------------------------------------------
{
	dbprintf(1,"DoConsumerNode %d -> (%d)\n", srcNodeID, nodeID);
}


//-----------------------------------------------------------------------------
int RandomInstructionGenerator::Run(unsigned sequenceLength)
//-----------------------------------------------------------------------------
{
	int idx = 0;
	while (idx<sequenceLength) {
		idx++;
		RunOnce();
	}
	EndCurrentChain();
	return 0;
}

//-----------------------------------------------------------------------------
void RandomInstructionGenerator::RunOnce(void)
//-----------------------------------------------------------------------------
{
	typedef enum { opNULL=0, opCREATE, opDELETE, opCHAIN, opNOOP, opUNKNOWN } opcode_t;
	const char* opcodeStr[] = { "", "create", "delete", "chain", "noop", "unknown" };
	LFSR_t prnum = lfsr.GetResult();	// pseudo-random number

	opcode_t opcode = opNULL;
	LFSR_t arg1;

	// determine the "raw" operation based on the prnum
	// (without knowing anything about the sequence or state)	
	if ((createRange>0) & (prnum <= createRange)) {
		opcode = opCREATE;
		arg1 = lfsr.Seed();
	} else if ((deleteRange>0) & (prnum-createRange <= deleteRange)) {
		opcode = opDELETE;
		prnum = lfsr.GetResult();
		arg1 = (prnum-1) % numNodes; // slightly biased toward the low end
	} else if (prnum-createRange-deleteRange <= chainRange) {
		opcode = opCHAIN;
		arg1 = (prnum-createRange-deleteRange-1) % numNodes;
	} else if (prnum-createRange-deleteRange-chainRange <= noopRange) {
		opcode = opNOOP;
	} else {
		opcode = opUNKNOWN;
	}

	dbprintf(3,"# %d %s %lu\n", prnum, opcodeStr[opcode], arg1);
	
	// now interpret the operation based on the current state
	if (opcode == opCHAIN) HandleChainOp(arg1);
	else if (opcode == opCREATE) HandleCreateOp();
	else if (opcode == opDELETE) HandleDeleteOp(arg1);
//	else {} do nothing
}

//-----------------------------------------------------------------------------
RandomInstructionGenerator::State RandomInstructionGenerator::GetState(void)
//-----------------------------------------------------------------------------
{
	State state = State(numNodes, debugLevel);
	state.feed = lfsr.Feed();
	state.seed = lfsr.Seed();
	state.deletedNodes = deletedNodes;
	return state;
}
/*
//-----------------------------------------------------------------------------
int main()
//-----------------------------------------------------------------------------
{
	RandomInstructionGenerator rig;
	rig.Run(100000);
}
*/
