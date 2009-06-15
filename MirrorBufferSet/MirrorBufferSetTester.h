//=============================================================================
//	$Id: MirrorBufferSetTester.h,v 1.1 2006/06/12 20:21:44 gallen Exp $
//-----------------------------------------------------------------------------
//	Tester of MirrorBufferSet
//=============================================================================


#ifndef MirrorBufferSetTester_h
#define MirrorBufferSetTester_h

#include "MirrorBufferSet.h"


class MirrorBufferSetTester : public MirrorBufferSet {
  public:
  	MirrorBufferSetTester(ulong bufferSz, ulong mirrorSz, int nBuffers)
  		: MirrorBufferSet(bufferSz,mirrorSz,nBuffers) { }
	int TestMirroring(unsigned long seed = 1) const;
};


#endif
