#include "PrimeSieve.h"
#include "PrimeSieveSource.h"

#define main5 main


int main1()
//	test just PrimeSieve::TryCandidate()
{
	printf("# Prime finder 1\n");

	PrimeSieve sieve;
	
	PrimeSieve::PrimeSieveT ppr = 2;
	sieve.TryCandidate(ppr);
	printf("%lu\n", ppr);
	
	for ( ppr=3; ppr<1000000; ppr+=2) {
		if ( sieve.TryCandidate(ppr) ) {
			printf("%lu\n", ppr);
		}
	}
}


int main2()
//	test PrimeSieve::TryCandidates(...)
{
	printf("# Prime finder 2\n");

	PrimeSieve sieve;
	
	PrimeSieve::PrimeSieveT ppr = 2;
	sieve.TryCandidate(ppr);
	printf("%lu\n", ppr);

	unsigned maxVal = 1000000;
	unsigned chunkLen = 1000;
	unsigned numPrimes = chunkLen;
	unsigned numPassed = chunkLen;
	PrimeSieve::PrimeSieveT candts[chunkLen];
	PrimeSieve::PrimeSieveT primes[chunkLen];
	PrimeSieve::PrimeSieveT passed[chunkLen];	

	for (unsigned long k=3; k<maxVal; k+=2*chunkLen) {
		// init candts array
		for (unsigned long j=0; j<chunkLen; j++)
			candts[j] = k+j*2;
	
		// call sieve
		numPrimes = chunkLen; numPassed = chunkLen;
		sieve.TryCandidates(candts,chunkLen, primes,numPrimes, passed,numPassed);
//		printf("# numPrimes %d, numPassed %d\n", numPrimes, numPassed);

		// print the primes
		for (unsigned long j=0; j<numPrimes; j++)
			printf("%lu\n", primes[j]);
	}
}


int main3()
//	test PrimeSieve::TryCandidates(...)
//	including passing
{
	printf("# Prime finder 3\n");

	PrimeSieve sieve(10);
	PrimeSieve sieve2;
	
	PrimeSieve::PrimeSieveT ppr = 2;
	sieve.TryCandidate(ppr);
	printf("%lu\n", ppr);

	unsigned maxVal = 1000000;
	unsigned chunkLen = 10;
	unsigned numPrimes = chunkLen;
	unsigned numPassed = chunkLen;
	PrimeSieve::PrimeSieveT candts[chunkLen];
	PrimeSieve::PrimeSieveT primes[chunkLen];
	PrimeSieve::PrimeSieveT passed[chunkLen];	

	for (unsigned long k=3; k<maxVal; k+=2*chunkLen) {
		// init candts array
		for (unsigned long j=0; j<chunkLen; j++)
			candts[j] = k+j*2;
#if 0
		printf("# candts[%d] = { ", chunkLen);
		for (unsigned long j=0; j<chunkLen; j++)
			printf("%d ", candts[j]);
		printf("}\n");
#endif
		// call sieve
		numPrimes = chunkLen; numPassed = chunkLen;
		sieve.TryCandidates(candts,chunkLen, primes,numPrimes, passed,numPassed);
//		printf("# sieve numPrimes %d, numPassed %d\n", numPrimes, numPassed);

		// print the primes
		for (unsigned long j=0; j<numPrimes; j++)
			printf("%lu\n", primes[j]);
		
		// ---- pass the passed downstream
		if (!numPassed) continue;
#if 0
		printf("# passed[%d] = { ", numPassed);
		for (unsigned long j=0; j<numPassed; j++)
			printf("%d ", passed[j]);
		printf("}\n");
#endif
		// use passed for candts and vice versa
		numPrimes = numPassed;
		sieve2.TryCandidates(passed,numPassed, primes,numPrimes, candts,numPassed);
//		printf("# sieve2 numPrimes %d, numPassed %d\n", numPrimes, numPassed);

		// print the primes
		for (unsigned long j=0; j<numPrimes; j++)
			printf("%lu\n", primes[j]);
	}
}


int main4()
//	test PrimeSieveSource
{
	printf("# Prime finder 4\n");

	{ PrimeSieveSource pss(1); }
	{ PrimeSieveSource pss(2); }
	{ PrimeSieveSource pss(3); }
	{ PrimeSieveSource pss(4); }
	{ PrimeSieveSource pss(5); }
	{ PrimeSieveSource pss(6); }
	{ PrimeSieveSource pss(7); }
	{ PrimeSieveSource pss(8); }
	
}


int main5()
//	test it all together!
{
	printf("# Prime finder 5\n");

	int numSourcePrimes = 5;
	PrimeSieve::PrimeSieveT sourcePrimes[numSourcePrimes];
	PrimeSieveSource primeSieveSource(numSourcePrimes, sourcePrimes);

	for (unsigned long j=0; j<numSourcePrimes; j++)
		printf("%lu\n", sourcePrimes[j]);
	
	PrimeSieve::PrimeSieveT maxVal = 100000000;
	unsigned numCandts = primeSieveSource.RoundLength();
	unsigned numPrimes = numCandts;
	unsigned numPassed = numCandts;
	
	PrimeSieve::PrimeSieveT candts[numCandts];
	PrimeSieve::PrimeSieveT primes[numCandts];
	PrimeSieve::PrimeSieveT passed[numCandts];
	
	PrimeSieve sieve(100);
	PrimeSieve sieve2;
	
	int done = 0;
	
	while (!done) {
		// fill up the candts array
		numCandts = primeSieveSource.GetNextRound(candts);
		printf("# source numCandts %d\n", numCandts);
		
		// call sieve
		sieve.TryCandidates(candts,numCandts, primes,numPrimes, passed,numPassed);
		printf("# sieve numPrimes %d, numPassed %d\n", numPrimes, numPassed);

		// print the primes
		for (unsigned long j=0; j<numPrimes; j++)
			printf("%lu\n", primes[j]);

		// ---- pass the passed downstream
		if (!numPassed) continue;
		
		// call the downstream sieve
		sieve2.TryCandidates(passed,numPassed, primes,numPrimes, candts,numPassed);
		printf("# sieve2 numPrimes %d, numPassed %d\n", numPrimes, numPassed);

		// print the primes
		for (unsigned long j=0; j<numPrimes; j++)
			printf("%lu\n", primes[j]);
		
		done = candts[numCandts-1]>=maxVal;
	}
}

