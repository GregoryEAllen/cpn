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


//-----------------------------------------------------------------------------
RandomInstructionGenerator::RandomInstructionGenerator()
//-----------------------------------------------------------------------------
:	lfsr(0xF82F,1)
{
	probabilityToCreateNode = 0.01;
	probabilityToDeleteNode = 0.01;
	
	printf("# lfsr of order %d, with range 1-%d\n", lfsr.Order(), lfsr.MaxVal());

	unsigned numNodes = 100;
	ComputeRanges(numNodes);
//		self.currentChain = []
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
int RandomInstructionGenerator::Run(unsigned sequenceLength)
//-----------------------------------------------------------------------------
{
	int idx = 0;
	while (idx<sequenceLength) {
		idx++;
		LFSR_t prnum = lfsr.GetResult();	// pseudo-random number
//		prnum = self.createRange+self.deleteRange+1

		printf("prnum %d\n",prnum);
	}
	
#if 0

			opcode = ''
			arg1 = ''
	
			//# determine the "raw" operation based on the prnum
			//# (without knowing anything about the sequence or state)	
			if (self.createRange>0) & (prnum <= self.createRange):
				opcode = 'create'
				arg1 = self.lfsr.seed
			elif (self.deleteRange>0) & (prnum-self.createRange <= self.deleteRange):
				opcode = 'delete'
				prnum = self.lfsr.GetResult()
				arg1 = (prnum-1) % self.numNodes //# slightly biased toward the low end
			elif (prnum-self.createRange-self.deleteRange <= self.chainRange):
				opcode = 'chain'
				arg1 = (prnum-self.createRange-self.deleteRange-1) % self.numNodes
			elif (prnum-self.createRange-self.deleteRange-self.chainRange <= self.noopRange):
				opcode = 'noop'
			else:
				opcode = 'unknown'
		
//#			print "#", prnum, opcode, arg1
		
			//# now interpret the operation based on the current state
			if opcode == 'chain':
				if self.currentChain.count(arg1)>0:
					//# already in the current chain, end the chain
					self.EndCurrentChain()
				elif self.deletedNodes.count(arg1)>0:
					//# can't add a deleted node, end the chain
					print "## not chaining deleted node %d" % (arg1)
//#					self.EndCurrentChain()
				else:
					self.currentChain.append(arg1)
			elif opcode == 'create':
				self.EndCurrentChain()
				self.DoCreateNode()
			elif opcode == 'delete':
				self.EndCurrentChain()
				self.DoDeleteNode(arg1)
			//#	else:
			//#		do nothing
		self.EndCurrentChain()
		
		print "# program statistics"
		print "# numNodes %d, len(deletedNodes) %d" % (self.numNodes, len(self.deletedNodes))
		print "# numChains %d, avgChainLength %f" % (self.numChains,self.avgChainLength)
		print "# numNodesCreated %d, numNodesDeleted %d" % (self.numNodesCreated,self.numNodesDeleted)
		print "# maxNumParallelChains %d" % (self.maxNumParallelChains)

		print "# deletedNodes =", self.deletedNodes

//#		print self.allChains

#endif

}



//-----------------------------------------------------------------------------
int main()
//-----------------------------------------------------------------------------
{
	RandomInstructionGenerator rig;
	rig.Run(100);
}