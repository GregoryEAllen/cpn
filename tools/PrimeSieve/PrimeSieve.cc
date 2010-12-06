//=============================================================================
//	PrimeSieve - class to sieve prime numbers
//	(Sieve of Eratosthenes)
//-----------------------------------------------------------------------------
//=============================================================================

#include <algorithm>
#include "PrimeSieve.h"

//-----------------------------------------------------------------------------
PrimeSieve::PrimeSieve(unsigned long maxNumPrimes_)
//-----------------------------------------------------------------------------
:	maxNumPrimes(maxNumPrimes_)
{
	primeSieve.reserve( maxNumPrimes ? maxNumPrimes : 10000 );
}


//-----------------------------------------------------------------------------
int PrimeSieve::TryCandidate(const PrimeSieveT primeCandidate)
// returns:
//	1 if pr is prime, and has been added to the sieve
//	-1 if pr passed the sieve, but the sieve is full (still possible prime)
//	0 if pr was stopped by the sieve
//-----------------------------------------------------------------------------
{
	if (!primeSieve.size()) {
		// if sieve is empty, this must be prime
		primeSieve.push_back( PrimeSieveEntry(primeCandidate) );
		push_heap(primeSieve.begin(), primeSieve.end());
		return 1;
	}
	
	while (1) {
		PrimeSieveT smallestSieveMultiple = primeSieve[0].multiple;	// smallest multiple
		if (primeCandidate > smallestSieveMultiple) {
			// increase the smallest multiple by its associated prime
            primeSieve[0].Advance();
			// and re-heap (could write a more efficient function)
			pop_heap(primeSieve.begin(), primeSieve.end());
			push_heap(primeSieve.begin(), primeSieve.end());
		} else if (primeCandidate == smallestSieveMultiple) {
			// stopped by the sieve
			return 0;
		} else {
			// passed the sieve, but are we full?
			if ( maxNumPrimes && (primeSieve.size()>=maxNumPrimes) )
				return -1;
			// insert our new prime and re-heap
			primeSieve.push_back( PrimeSieveEntry(primeCandidate) );
			push_heap(primeSieve.begin(), primeSieve.end());
			return 1;
		}
	}
}


//-----------------------------------------------------------------------------
void PrimeSieve::TryCandidatesSimple(const PrimeSieveT* candidates, unsigned numCandidates,
				 PrimeSieveT* primes, unsigned& numPrimes,
				 PrimeSieveT* passed, unsigned& numPassed)
//	primes are the candidates which were prime and added to the sieve
//	passed are the candidates which passed the sieve (still possibly prime)
//  both should be >= numCandidates
//	numPrimes and numPassed are outputs set to the number of primes/passed found
//-----------------------------------------------------------------------------
//	a simple (but slow) loop, repeatedly calling TryCandidate(...)
{
//	printf("# PrimeSieve::TryCandidatesSimple(0x%08X)\n", this);

	numPrimes = 0;
	numPassed = 0;
	
	for (int i=0; i<numCandidates; i++) {
		switch ( TryCandidate(candidates[i]) ) {
			case 0: break; // stopped by the sieve
			case 1: // prime added to sieve
				primes[numPrimes++] = candidates[i];
				break;
			case -1: // potential prime passed full sieve
				passed[numPassed++] = candidates[i];
				break;
		}
	}
}


//-----------------------------------------------------------------------------
void PrimeSieve::TryCandidates(const PrimeSieveT* candidates, unsigned numCandidates,
				 PrimeSieveT* primes, unsigned& numPrimes,
				 PrimeSieveT* passed, unsigned& numPassed)
//	primes are the candidates which were prime and added to the sieve
//	passed are the candidates which passed the sieve (still possibly prime)
//  both should be >= numCandidates
//	numPrimes and numPassed are outputs set to the number of primes/passed found
//-----------------------------------------------------------------------------
//	I still think this could be made faster by checking all the primeCandidates
//  against smallestSieveMultiple instead of double looping
{
//	printf("# PrimeSieve::TryCandidates(0x%08X)\n", this);
	if (!numCandidates) return;

	numPrimes = 0;
	numPassed = 0;
	
	if (!primeSieve.size()) {
		// if sieve is empty, first entry must be prime
		PrimeSieveT primeCandidate = candidates[0];
		primeSieve.push_back( PrimeSieveEntry(primeCandidate) );
		push_heap(primeSieve.begin(), primeSieve.end());
		primes[numPrimes++] = primeCandidate;
		candidates++; numCandidates--;
	}

	for (int i=0; i<numCandidates; i++) {
		PrimeSieveT primeCandidate = candidates[i];
		PrimeSieveT smallestSieveMultiple = primeSieve[0].multiple;	// smallest multiple
		while (primeCandidate > smallestSieveMultiple) {
			// increase the smallest multiple by its associated prime
			primeSieve[0].Advance();
			// and re-heap (could write a more efficient function)
			pop_heap(primeSieve.begin(), primeSieve.end());
			push_heap(primeSieve.begin(), primeSieve.end());
			smallestSieveMultiple = primeSieve[0].multiple;
		}
		if (primeCandidate < smallestSieveMultiple) {
			// passed the sieve, but are we full?
			if ( maxNumPrimes && (primeSieve.size()>=maxNumPrimes) ) {
				// potential prime passed full sieve
				passed[numPassed++] = primeCandidate;
			} else {
				// insert our new prime and re-heap
				primeSieve.push_back( PrimeSieveEntry(primeCandidate) );
				push_heap(primeSieve.begin(), primeSieve.end());
				primes[numPrimes++] = primeCandidate;
			}
		} //else ;// stopped by the sieve
	}
}


#if 0

using namespace std;
#include <iostream>

//-----------------------------------------------------------------------------
void PrimeSieve::PrintPrimeSieve(void)
//-----------------------------------------------------------------------------
{
	vector<PrimeSieveEntry>::iterator i;
	cout << "PrimeSieve = [" << endl;
	for (i=primeSieve.begin(); i<primeSieve.end(); i++) {
		cout << "\t" << (*i).multiple << "\t" << (*i).prime << endl;
	}
	cout << "];" << endl;
}
#endif
