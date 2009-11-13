//=============================================================================
//  $Id:$
//-----------------------------------------------------------------------------
//  RandomInstructionGenerator class for PN random case study
//-----------------------------------------------------------------------------
//  Computational Process Networks class library
//  Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//  This library is free software; you can redistribute it and/or modify it
//  under the terms of the GNU Library General Public License as published
//  by the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Library General Public License for more details.
//
//  The GNU Public License is available in the file LICENSE, or you
//  can write to the Free Software Foundation, Inc., 59 Temple Place -
//  Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//  World Wide Web at http://www.fsf.org.
//=============================================================================

#ifndef RandomInstructionGenerator_h
#define RandomInstructionGenerator_h


#include "LFSR.h"
#include <deque>


class RandomInstructionGenerator
{
  public:
    typedef LFSR::LFSR_t LFSR_t;
    static const LFSR_t DEFAULT_SEED;
    static const LFSR_t DEFAULT_FEED;

    struct State {
        State() {}

        // constructor for copying state for new nodes
        State(unsigned mid, int dbglvl, std::deque<unsigned> ln)
            : feed(DEFAULT_FEED), seed(DEFAULT_SEED), maxID(mid),
            debugLevel(dbglvl), liveNodes(ln) {}

        LFSR_t feed, seed;
        unsigned maxID;
        int debugLevel;
        std::deque<unsigned> liveNodes;
    };

    RandomInstructionGenerator(unsigned numNodes);
    RandomInstructionGenerator(const State& state);
    virtual ~RandomInstructionGenerator() {}
    State GetState(void);
    void SetState(const State &state);
    int Run(unsigned sequenceLength = 10);
    void RunOnce(void);

  protected:
    LFSR lfsr;
    
    float probabilityToCreateNode, probabilityToDeleteNode;
    unsigned maxID;
    unsigned createRange, deleteRange, chainRange, noopRange;
    
    std::deque<unsigned> currentChain;
    std::deque<unsigned> liveNodes;

    void EndCurrentChain(void);

    void ComputeRanges();
    void HandleChainOp(unsigned nodeID);
    void HandleCreateOp(void);
    void HandleDeleteOp(unsigned nodeID);
    
    int debugLevel;
    virtual int dbprintf(int dbLevel, const char *fmt, ...);
    
    void Initialize(int dbglvl, float probToCreateNode,
        float probToDeleteNode, unsigned startNumNodes);

    // functions to be overridden
    virtual void DoCreateNode(unsigned newNodeID, unsigned creatorNodeID);
    virtual void DoDeleteNode(unsigned nodeID);
    virtual void DoProducerNode(unsigned nodeID, unsigned dstNodeID);
    virtual void DoTransmuterNode(unsigned nodeID, unsigned srcNodeID, unsigned dstNodeID);
    virtual void DoConsumerNode(unsigned nodeID, unsigned srcNodeID);
};


#endif
