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

#ifndef RandomInstructionGenerator_h
#define RandomInstructionGenerator_h


#include "LFSR.h"
#include <deque>


class RandomInstructionGenerator
{
  public:
	typedef LFSR::LFSR_t LFSR_t;
	RandomInstructionGenerator();
	int Run(unsigned sequenceLength = 10);

  protected:
	LFSR lfsr;
	
	float probabilityToCreateNode, probabilityToDeleteNode;
	unsigned numNodes;
	unsigned createRange, deleteRange, chainRange, noopRange;
	
	std::deque<unsigned> currentChain;
	std::deque<unsigned> deletedNodes;

	void EndCurrentChain(void);

	void ComputeRanges(unsigned nmNodes);
	void HandleChainOp(unsigned nodeID);
	void HandleCreateOp(void);
	void HandleDeleteOp(unsigned nodeID);
};


#endif
