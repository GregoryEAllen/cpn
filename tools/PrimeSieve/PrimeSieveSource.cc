//=============================================================================
//	PrimeSieveSource - class to source potential primes for the sieve
//	Uses a prime wheel
//-----------------------------------------------------------------------------
//=============================================================================

#include "PrimeSieveSource.h"
#include <assert.h>

#include <stdio.h>

//-----------------------------------------------------------------------------
PrimeSieveSource::PrimeSieveSource(unsigned long numPrimes, PrimeSieveT* primes)
//-----------------------------------------------------------------------------
:	whichRound(0), baseValue(0)
{
//	printf("# PrimeSieveSource::PrimeSieveSource(%d)\n", numPrimes);
	assert(numPrimes>0);
	assert(numPrimes<=8);

	// because the potential list is so small, we just precompute to save memory
	unsigned long smallPrimes[] = { 2, 3, 5, 7, 11, 13, 19, 23 };
	unsigned long roundLengths[] = { 1, 2, 8, 48, 480, 5760, 92160, 1658880 };
	
	roundLength = roundLengths[ numPrimes-1 ];
	buffer = new PrimeSieveT[roundLength];
	
	assert(buffer);
	
//	printf("buffer(1:%d) = [\n", roundLength);
	
	// always begin with 1
	unsigned long idx = 0;
	buffer[idx++] = 1;
//	printf(" %d\n", buffer[idx-1]);

	PrimeSieve primeSieve(numPrimes);
	
	// get the base case of 2
	primeSieve.TryCandidate(2);
	if (primes) *primes++ = 2;
	cumprod = 2;
		
	for (unsigned long k=3; k<cumprod || (primeSieve.NumPrimes()<numPrimes); k+=2) {
		int result = primeSieve.TryCandidate(k);
		if (result==1) {
			cumprod *= k;
			if (primes) *primes++ = k;
		} else if (result==-1) {	// it passed the sieve
			buffer[idx++] = k;
//			printf(" %d\n", buffer[idx-1]);
		}
	}
//	printf("];\n", roundLength);
	
//	printf("idx %d, roundLength %d\n",idx,roundLength);
}


//-----------------------------------------------------------------------------
PrimeSieveSource::~PrimeSieveSource(void)
//-----------------------------------------------------------------------------
{
	delete[] buffer;
}

//-----------------------------------------------------------------------------
unsigned PrimeSieveSource::GetNextRound(PrimeSieveT* dest)
//-----------------------------------------------------------------------------
{
	if (!whichRound++) {
		// don't copy the 1 in the first round
		for (unsigned long k=0; k<roundLength-1; k++)
			dest[k] = buffer[k+1];
		return roundLength-1;
	}
	
	// all the remaining rounds
	baseValue += cumprod;
	for (unsigned long k=0; k<roundLength; k++)
		dest[k] = buffer[k] + baseValue;
	return roundLength;
}
