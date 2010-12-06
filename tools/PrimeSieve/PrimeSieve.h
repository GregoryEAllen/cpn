//=============================================================================
//  PrimeSieve - class to sieve prime numbers
//  (Sieve of Eratosthenes)
//-----------------------------------------------------------------------------
//=============================================================================

#ifndef PrimeSieve_h
#define PrimeSieve_h

#include <vector>

class PrimeSieve {
  public:
    typedef unsigned long long PrimeSieveT;
  protected:
    struct PrimeSieveEntry {
        PrimeSieveT multiple;
        PrimeSieveT advancer[2];
        PrimeSieveEntry(PrimeSieveT pr)
            : multiple(pr)
        {
            if (pr > 3) {
                advancer[0] = pr*4;
                advancer[1] = pr*2;
            } else if (pr > 2) {
                advancer[0] = pr*2;
                advancer[1] = pr*2;
            } else {
                advancer[0] = pr;
                advancer[1] = pr;
            }
            Advance();
        }

        bool operator<(const PrimeSieveEntry& __y)
            { return this->multiple > __y.multiple; } // reverse sort

        void Advance() {
            multiple += advancer[0];
            std::swap(advancer[0], advancer[1]);
        }
    };

    // one entry in the vector for each found prime number
    std::vector<PrimeSieveEntry> primeSieve;
    unsigned long maxNumPrimes;

    void PrintPrimeSieve(void);

  public:
    PrimeSieve(unsigned long maxNumPrimes=0);
    //  sieve never fills up for maxNumPrimes=0 (memory permitting)

    unsigned long NumPrimes(void) { return primeSieve.size(); }
    // currently in the sieve

    int TryCandidate(const PrimeSieveT pr);
    // returns:
    //  1 if pr is prime, and has been added to the sieve
    //  -1 if pr passed the sieve, but the sieve is full (still possible prime)
    //  0 if pr was stopped by the sieve

    void TryCandidates(const PrimeSieveT* candidates, unsigned numCandidates,
                 PrimeSieveT* primes, unsigned& numPrimes,
                 PrimeSieveT* passed, unsigned& numPassed);
    //  primes are the candidates which were prime and added to the sieve
    //  passed are the candidates which passed the sieve (still possibly prime)
    //  both should be >= numCandidates
    //  numPrimes and numPassed are outputs set to the number of primes/passed found

    void TryCandidatesSimple(const PrimeSieveT* candidates, unsigned numCandidates,
                 PrimeSieveT* primes, unsigned& numPrimes,
                 PrimeSieveT* passed, unsigned& numPassed);
    //  a simple (but slow) loop, repeatedly calling TryCandidate(...)
};

#endif
