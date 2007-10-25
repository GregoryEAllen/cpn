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


//-----------------------------------------------------------------------------
RandomInstructionGenerator::RandomInstructionGenerator()
//-----------------------------------------------------------------------------
:	lfsr(0xF82F,1)
{
	debugLevel = 2;
	
	probabilityToCreateNode = 0.01;
	probabilityToDeleteNode = 0.01;
	
	dbprintf(2,"# lfsr of order %d, with range 1-%d\n", lfsr.Order(), lfsr.MaxVal());

	unsigned numNodes = 100;
	ComputeRanges(numNodes);
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
		dbprintf(1,"create chain: ");
		while (currentChain.size()>0) {
			dbprintf(1,"%lu ", currentChain[0]);
			currentChain.pop_front();
		}
		dbprintf(1,"\n");
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
	unsigned responsibleNode = 0;
	// this could probably be much faster by sorting first
	while ( find( deletedNodes.begin(), deletedNodes.end(), responsibleNode ) != deletedNodes.end() ) {
		responsibleNode += 1;
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
	dbprintf(1,"DoCreateNode %d, responsibleNode %d\n", newNodeID, responsibleNode);
	dbprintf(2,"## len(deletedNodes) %d\n", deletedNodes.size());
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
	dbprintf(1,"DoDeleteNode %d\n", nodeID);
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
int RandomInstructionGenerator::Run(unsigned sequenceLength)
//-----------------------------------------------------------------------------
{
	typedef enum { opNULL=0, opCREATE, opDELETE, opCHAIN, opNOOP, opUNKNOWN } opcode_t;
	const char* opcodeStr[] = { "", "create", "delete", "chain", "noop", "unknown" };
	
	int idx = 0;
	while (idx<sequenceLength) {
		idx++;
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
//		else {} do nothing
	}
	EndCurrentChain();
	return 0;
}



//-----------------------------------------------------------------------------
int main()
//-----------------------------------------------------------------------------
{
	RandomInstructionGenerator rig;
	rig.Run(100000);
}