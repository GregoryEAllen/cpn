//=============================================================================
//	$Id: MirrorBufferSetTester.cc,v 1.1 2006/06/12 20:21:44 gallen Exp $
//-----------------------------------------------------------------------------
//	Tester of MirrorBufferSet
//=============================================================================


#include "MirrorBufferSetTester.h"

#include <stdio.h>
#include <stdlib.h>


//-----------------------------------------------------------------------------
//	flags and definitions for debugging printouts
//-----------------------------------------------------------------------------
#ifndef DEBUG
#define DEBUG 2
#endif

#if DEBUG >= 1
    #define fprintf1(p) fprintf p
#else
    #define fprintf1(p)
#endif

#if DEBUG >= 2
    #define fprintf2(p) fprintf p
#else
    #define fprintf2(p)
#endif


//-----------------------------------------------------------------------------
int MirrorBufferSetTester::TestMirroring(unsigned long seed) const
//-----------------------------------------------------------------------------
{
	int buf,i;

	if (!bufferBase) {
		fprintf(stderr,"Failed to create MirrorBufferSet\n");
		return -1;
	}

	// set the random seed
	srand(seed);

	// fill in all of the buffers
	fprintf2((stderr,"Writing %d bytes to %d buffers\n", bufferSize, numBuffers));
	for (buf=0; buf<numBuffers; buf++) {
		char* bufBase = (char*)bufferBase + buf*(bufferSize+mirrorSize);
		for (int i=0; i<bufferSize; i++) {
			bufBase[i] = (char) rand();
		}
	}

	srand(seed);
	long bufErr = 0;
	long mirErr = 0;

	// verify the buffers and the mirrors
	fprintf2((stderr,"Verifying... "));
	fflush(stdout);
	for (buf=0; buf<numBuffers; buf++) {
		char* bufBase = (char*)bufferBase + buf*(bufferSize+mirrorSize);
		char* mirBase = bufBase + bufferSize;
		for (i=0; i<mirrorSize; i++) {
			char theVal = (char) rand();
			if (theVal != bufBase[i]) {
				bufErr++;
				printf("### Error in buffer %d @ idx %d: 0x%02X != 0x%02X\n",
					buf, i, bufBase[i], theVal);
			}
			if (theVal != mirBase[i]) {
				mirErr++;
				printf("### Error in mirror %d @ idx %d: 0x%02X != 0x%02X\n",
					buf, i, mirBase[i], theVal);
			}
		}
		for (; i<bufferSize; i++) {
			char theVal = (char) rand();
			if (theVal != bufBase[i]) {
				bufErr++;
				printf("### Error in buffer %d @ idx %d: 0x%02X != 0x%02X\n",
					buf, i, bufBase[i], theVal);
			}
		}
	}
	if (bufErr || mirErr)
		fprintf2((stderr,"Error\n"));
	else
		fprintf2((stderr,"Passed\n"));

	// now fill in just the mirror parts
	fprintf2((stderr,"Writing %d bytes to %d mirrors\n", mirrorSize, numBuffers));
	srand(seed);
	for (buf=0; buf<numBuffers; buf++) {
		char* bufBase = (char*)bufferBase + buf*(bufferSize+mirrorSize);
		char* mirBase = bufBase + bufferSize;
		for (int i=0; i<mirrorSize; i++) {
			mirBase[i] = (char) rand();
		}		
	}

	// and verify the buffer bases
	fprintf2((stderr,"Verifying... "));
	fflush(stdout);
	srand(seed);
	for (buf=0; buf<numBuffers; buf++) {
		char* bufBase = (char*)bufferBase + buf*(bufferSize+mirrorSize);
		char* mirBase = bufBase + bufferSize;
		for (int i=0; i<mirrorSize; i++) {
			char theVal = (char) rand();
			if (theVal != bufBase[i]) {
				bufErr++;
				fprintf(stderr,"### Error in buffer %d @ idx %d: 0x%02X != 0x%02X\n",
					buf, i, bufBase[i], theVal);
			}
			if (theVal != mirBase[i]) {
				mirErr++;
				fprintf(stderr,"### Error in mirror %d @ idx %d: 0x%02X != 0x%02X\n",
					buf, i, mirBase[i], theVal);
			}
		}
	}
	if (bufErr || mirErr) {
		fprintf2((stderr,"Error\n"));
		fprintf2((stderr,"bufErr %d, mirErr %d\n", bufErr, mirErr));
	} else
		fprintf2((stderr,"Passed\n"));

	return bufErr || mirErr;
}

