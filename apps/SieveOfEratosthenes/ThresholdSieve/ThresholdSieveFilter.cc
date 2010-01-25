/** \file
 */

#include "ThresholdSieveFilter.h"
#include "NodeFactory.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "ToString.h"
#include "PrimeSieve.h"
#include "Assert.h"
#include <cmath>
#include <stdexcept>
#include <sstream>

#if _DEBUG
#include <cstdio>
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

using CPN::shared_ptr;

typedef ThresholdSieveOptions::NumberT NumberT;

class FilterFactory : public CPN::NodeFactory {
public:
    FilterFactory() : CPN::NodeFactory(THRESHOLDSIEVEFILTER_TYPENAME) {}
    shared_ptr<CPN::NodeBase> Create(CPN::Kernel &ker, const CPN::NodeAttr &attr) {
        ASSERT(attr.GetArg().GetSize() == sizeof(ThresholdSieveOptions));
        ThresholdSieveOptions *opts = (ThresholdSieveOptions*)attr.GetArg().GetBuffer();
        return shared_ptr<CPN::NodeBase>(new ThresholdSieveFilter(ker, attr, *opts));
    }
};

const NumberT *GetDequeueCount(CPN::QueueReaderAdapter<NumberT> &in, unsigned &incount) {
    const NumberT *inbuff = in.GetDequeuePtr(incount);
    if (!inbuff) {
        incount = in.Count();
        if (incount != 0) {
            inbuff = in.GetDequeuePtr(incount);
        }
    }
    return inbuff;
}

void ThresholdSieveFilter::ReportCandidates(
        const ThresholdSieveOptions::NumberT *inbuff, unsigned incount,
        ThresholdSieveOptions::NumberT *primes, unsigned numPrimes,
        ThresholdSieveOptions::NumberT *passed, unsigned numPassed)
{
    std::ostringstream oss;
    oss << GetName() << "\nin ";
    for (unsigned i = 0; i < incount; ++i) {
        oss << inbuff[i] << " ";
    }
    if (numPrimes != 0) {
        oss << "\nprimes ";
        for (unsigned i = 0; i < numPrimes; ++i) {
            oss << primes[i] << " ";
        }
    }
    if (numPassed != 0) {
        oss << "\npassed ";
        for (unsigned i = 0; i < numPassed; ++i) {
            oss << passed[i] << " ";
        }
    }
    puts(oss.str().c_str());
}

void ThresholdSieveFilter::Process() {
    DEBUG("%s started\n", GetName().c_str());
    CPN::QueueReaderAdapter<NumberT> in = GetReader(IN_PORT);
    CPN::QueueWriterAdapter<NumberT> out = GetWriter(CONTROL_PORT);
    const unsigned long threshold = opts.threshold;
    const NumberT cutoff = (NumberT)(ceil(sqrt(opts.maxprime)));
    NumberT buffer[threshold];
    PrimeSieve sieve(opts.primesPerFilter);
    unsigned numPrimes = 0;
    unsigned numPassed = 0;
    unsigned incount = threshold;
    bool loop = true;
    while (loop && (numPassed == 0) ) {
        incount = threshold;
        const NumberT *inbuff = GetDequeueCount(in, incount);
        if (!inbuff) {
            loop = false;
        } else {
            NumberT *outbuff = out.GetEnqueuePtr(incount);
            sieve.TryCandidates(inbuff, incount, outbuff, numPrimes, buffer, numPassed);
#if _DEBUG
            ReportCandidates(inbuff, incount, outbuff, numPrimes, buffer, numPassed);
#endif
            out.Enqueue(numPrimes);
            in.Dequeue(incount);
            DEBUG("%s processed primes %u -> %u (%u)\n", GetName().c_str(), incount, numPrimes, numPassed);
        }
    }
    if (loop) {
        if (buffer[0] <= cutoff) {
            out.Release();
            CreateNewFilter();
            DEBUG("%s created new filter\n", GetName().c_str());
            out = GetWriter(OUT_PORT);
        }
        out.Enqueue(buffer, numPassed);
    }
    while (loop) {
        incount = threshold;
        const NumberT *inbuff = GetDequeueCount(in, incount);
        if (!inbuff) {
            loop = false;
        } else {
            NumberT *outbuff = out.GetEnqueuePtr(incount);
            sieve.TryCandidates(inbuff, incount, buffer, numPrimes, outbuff, numPassed);
#if _DEBUG
            ReportCandidates(inbuff, incount, buffer, numPrimes, outbuff, numPassed);
#endif
            ASSERT(numPrimes == 0);
            out.Enqueue(numPassed);
            in.Dequeue(incount);
            DEBUG("%s processed candidates %u -> %u (%u)\n", GetName().c_str(), incount, numPassed, numPrimes);
        }
    }
    out.Release();
    in.Release();
    DEBUG("%s stopped\n", GetName().c_str());
}

extern "C" shared_ptr<CPN::NodeFactory> cpninitThresholdSieveFilterType(void);

shared_ptr<CPN::NodeFactory> cpninitThresholdSieveFilterType(void) {
    return (shared_ptr<CPN::NodeFactory>(new FilterFactory));
}

