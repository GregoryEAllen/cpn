//=============================================================================
//	$Id: main.cc,v 1.1 2006/06/12 20:21:44 gallen Exp $
//-----------------------------------------------------------------------------
//=============================================================================


#include "MirrorBufferSetTester.h"
#include <stdio.h>
#include <stdlib.h>


int main (int argc, char **argv) __attribute__((weak));
//-----------------------------------------------------------------------------
int main(int,char**)
//-----------------------------------------------------------------------------
{
//	fprintf(stderr,"### main()\n");

	int supported = MirrorBufferSet::Supported();

	fprintf(stderr,"MirrorBufferSet is ");
	
	switch (supported) {
		case MirrorBufferSet::eNotSupported:
			fprintf(stderr,"not supported ");
			break;
		case MirrorBufferSet::eSupportedPosixShm:
			fprintf(stderr,"supported (with POSIX Shared Memory) ");
			break;
		case MirrorBufferSet::eSupportedTmpFile:
			fprintf(stderr,"supported (with a temporary file) ");
			break;
	}
	fprintf(stderr,"on this platform.\n");
	
	unsigned long pageSize = MirrorBufferSet::PageSize();
	fprintf(stderr,"The page size is %lu bytes.\n", pageSize);

	MirrorBufferSetTester mbs(2*pageSize,1*pageSize,4);

	unsigned long bufferSize = mbs.BufferSize();
	unsigned long mirrorSize = mbs.MirrorSize();
	unsigned long numBuffers = mbs.NumBuffers();

	fprintf(stderr,"bufferSize = %lu, mirrorSize = %lu, numBuffers = %lu\n",
		bufferSize, mirrorSize, numBuffers );

	int err = mbs.TestMirroring();
	if (err) {
		fprintf(stderr,"MirrorBufferSet::TestMirroring() returned error %d\n",err);
	}
	return err;
}
