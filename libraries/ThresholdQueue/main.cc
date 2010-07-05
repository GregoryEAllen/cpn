//=============================================================================
//	TestThresholdQueue
//=============================================================================

#include "ThresholdQueue.h"

#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define PrintDebug printf
#define PrintDebug1
#define PrintDebug2


//-----------------------------------------------------------------------------
int TestThresholdQueueBase(int argc, char* argv[])
//-----------------------------------------------------------------------------
{
	ThresholdQueueBase thq(sizeof(float), 20, 10);
	thq.Reset();
}

#if 1
//-----------------------------------------------------------------------------
int TestThresholdQueue1(int argc, char* argv[])
//-----------------------------------------------------------------------------
{
	long numWriteElems = 100000;
	
	if (argc>1)
		numWriteElems = atol(argv[1]);
	
	printf("### %s testing with %d elements...\n", argv[0], numWriteElems);

	unsigned int seed = 1;
	srand(seed);
	
	const int kQueueSize = 2*2048;
	int threshSize = 2048;
	int numChannels = 3;
	bool useMBS = 0;
	
	ThresholdQueueAttr	attr(kQueueSize,threshSize,numChannels,useMBS);
	ThresholdQueue<long>	theQueue(attr);
	
	if ( theQueue.QueueLength() != kQueueSize) {
		PrintDebug("theQueue resized to %d\n", theQueue.QueueLength());
	}

	if (theQueue.MaxThreshold() != threshSize) {
		threshSize = theQueue.MaxThreshold();
		PrintDebug("theQueue MaxThreshold changed to %d\n", threshSize);
	}

	PrintDebug2("count = %d, freespace = %d\n", theQueue.Count(), theQueue.Freespace());
	
	long errors = 0;
	long writeVal = 0;
	long readVal = 0;

	while (readVal<numWriteElems) {
		// verify and remove a random-sized chunk
		int deqThresh = rand() % (theQueue.MaxThreshold()) + 1; 
		int deqSize = deqThresh/2; 
		PrintDebug1("deqThresh = %d, deqSize = %d\n", deqThresh, deqSize);

		if (theQueue.Count() >= deqThresh) {
			for (int chan=0; chan<numChannels; chan++) {
				const long* readPtr = theQueue.GetDequeuePtr(deqSize, chan);
				assert( readPtr == theQueue.GetDequeuePtr(deqSize)
					+ chan * theQueue.ChannelStride() );
				long myReadVal = readVal;
				PrintDebug2("reading %d elements @ 0x%08X\n", deqThresh, readPtr);
				for (int i=0; i<deqThresh; i++) {
					register long val = myReadVal++ * (chan+1);
					if (readPtr[i] != val) {
						PrintDebug1("### Error: read %d, expected %d\n", readPtr[i], val);
						errors++;
					}
				}
			}
			readVal += deqSize;
			theQueue.Dequeue(deqSize);
			PrintDebug1("verified %d elements, dequeued %d elements\n", deqThresh, deqSize);
		}

		if (writeVal>=numWriteElems*2) continue;

		// write a random-sized chunk
		int enqSize = rand() % (theQueue.MaxThreshold()) + 1;
		PrintDebug2("enqSize = %d\n", enqSize);

		if (theQueue.Freespace() >= enqSize) {
			for (int chan=0; chan<numChannels; chan++) {
				long myWriteVal = writeVal;
				long* writePtr = theQueue.GetEnqueuePtr(enqSize, chan);
				assert( writePtr == theQueue.GetEnqueuePtr(enqSize)
					+ chan * theQueue.ChannelStride() );
				PrintDebug2("writing %d elements @ 0x%08X\n", enqSize, writePtr);
				for (int i=0; i<enqSize; i++)
					writePtr[i] = myWriteVal++ * (chan+1);
			}
			writeVal += enqSize;
			theQueue.Enqueue(enqSize);
			PrintDebug1("enqueued %d elements\n", enqSize);
		} else {
			unsigned long newSize = theQueue.Count() + enqSize;
			if (theQueue.QueueLength() <= kQueueSize*5)
				if (!(rand() % 20)) {// throw the dice, and grow sometimes
					threshSize += 200;
					theQueue.Grow(newSize, threshSize);
					if ( theQueue.QueueLength() != kQueueSize || theQueue.MaxThreshold() != threshSize) {
						PrintDebug("theQueue size %lu, maxThreshold %lu\n", theQueue.QueueLength(), theQueue.MaxThreshold());
					}
				}
		}
	}
	
	PrintDebug("theQueue.ElementsEnqueued() = %d\n", theQueue.ElementsEnqueued());
	PrintDebug("theQueue.ElementsDequeued() = %d\n", theQueue.ElementsDequeued());
	
	if (errors)
		PrintDebug("### %d errors detected!\n", errors);
	else
		PrintDebug("No errors detected.\n", errors);
}
#endif

int main (int argc, char **argv) __attribute__((weak));
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
//-----------------------------------------------------------------------------
{
	return TestThresholdQueue1(argc,argv);
	return TestThresholdQueueBase(argc,argv);
}

