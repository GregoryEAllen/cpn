//=============================================================================
//	PrimeSieveSource - class to source potential primes for the sieve
//	Uses a prime wheel
//-----------------------------------------------------------------------------
//=============================================================================

#ifndef PrimeSieveSource_h
#define PrimeSieveSource_h

#include "PrimeSieve.h"

class PrimeSieveSource {
  public:
	typedef PrimeSieve::PrimeSieveT PrimeSieveT;

  protected:
	unsigned long	whichRound;
	PrimeSieveT 	baseValue;
	unsigned long	cumprod;
	unsigned long	roundLength;
	PrimeSieveT*	buffer;

  public:
	PrimeSieveSource(unsigned long numPrimes, PrimeSieveT* primes=0);
	// The number of primes that we take a product of to create our wheel
	// RoundLength() is a percentage of the cumulative product of the primes,
	//	which grows extremely quickly
	// RoundLength() = [ 1, 2, 8, 48, 480, 5760, 92160, 1658880 ]
   ~PrimeSieveSource(void);

	unsigned RoundLength(void) { return roundLength; }
	// The (max) number of potential primes issued by GetNextRound
	
	unsigned GetNextRound(PrimeSieveT* dest);
	// copy the next RoundLength() potential primes to dest[]
	// and return the number that was copied
};

#endif
