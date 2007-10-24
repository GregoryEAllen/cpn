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
#include <algorithm>


//-----------------------------------------------------------------------------
RandomInstructionGenerator::RandomInstructionGenerator()
//-----------------------------------------------------------------------------
:	lfsr(0xF82F,1)
{
	probabilityToCreateNode = 0.0;
	probabilityToDeleteNode = 0.0;
	
	printf("# lfsr of order %d, with range 1-%d\n", lfsr.Order(), lfsr.MaxVal());

	unsigned numNodes = 100;
	ComputeRanges(numNodes);
	
//		self.deletedNodes = []
}


//-----------------------------------------------------------------------------
void RandomInstructionGenerator::ComputeRanges(unsigned nmNodes)
//-----------------------------------------------------------------------------
{
	numNodes = nmNodes;
	LFSR_t maxVal = lfsr.MaxVal();
	
	createRange = unsigned( floor(maxVal*probabilityToCreateNode+0.5) );
	deleteRange = unsigned( floor(maxVal*probabilityToDeleteNode+0.5) );

	printf("# numNodes = %d\n", numNodes);
	printf("# createRange = %d, deleteRange = %d\n", createRange, deleteRange);

	chainRange = ((maxVal-createRange-deleteRange)/numNodes) * numNodes; // int math
	noopRange = maxVal-createRange-deleteRange-chainRange;

	printf("# chainRange = %d, noopRange = %d\n", chainRange, noopRange);
	
//	unsigned totalRange = createRange + deleteRange + chainRange + noopRange;
//	printf("# totalRange = %d\n", totalRange);

	if (chainRange==0) {
		printf("Error: lfsr seed order is too small");
		exit(-1);
	}
}


//-----------------------------------------------------------------------------
void RandomInstructionGenerator::EndCurrentChain(void)
//-----------------------------------------------------------------------------
{
	if (currentChain.size()<=1) {
		printf("# discarding a chain of length %d\n", currentChain.size());
		currentChain.clear();
	} else {
		printf("create chain: ");
		while (currentChain.size()>0) {
			printf("%lu ", currentChain[0]);
			currentChain.pop_front();
		}
		printf("\n");
	}
}


//-----------------------------------------------------------------------------
void RandomInstructionGenerator::DoCreateNode(void)
//-----------------------------------------------------------------------------
{
}


//-----------------------------------------------------------------------------
void RandomInstructionGenerator::DoDeleteNode(unsigned nodeID)
//-----------------------------------------------------------------------------
{
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

//		printf("# %d %s %lu\n", prnum, opcodeStr[opcode], arg1);
		
		// now interpret the operation based on the current state
		if (opcode == opCHAIN) {
			if ( find( currentChain.begin(), currentChain.end(), arg1 ) != currentChain.end()  ) {
				// already in the current chain, end the chain
				EndCurrentChain();
			} else if ( find( deletedNodes.begin(), deletedNodes.end(), arg1 ) != deletedNodes.end()  ) {
				// can't add a deleted node, end the chain
				printf("## not chaining deleted node %lu\n", arg1);
//				EndCurrentChain()
			} else {
				currentChain.push_back(arg1);
			}
		} else if (opcode == opCREATE) {
			EndCurrentChain();
			DoCreateNode();
		} else if (opcode == opDELETE) {
			EndCurrentChain();
			DoDeleteNode(arg1);
		} // else {} do nothing
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