//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \author John Bridgman
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
#include <cstdio>

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

#if 0
#define REPORT(fmt, ...) printf(fmt, ## __VA_ARGS)
#else
#define REPORT(fmt, ...)
#endif

using CPN::shared_ptr;

typedef ThresholdSieveOptions::NumberT NumberT;

CPN_DECLARE_NODE_FACTORY(ThresholdSieveFilter, ThresholdSieveFilter);

ThresholdSieveFilter::ThresholdSieveFilter(CPN::Kernel &ker, const CPN::NodeAttr &attr)
    : CPN::NodeBase(ker, attr)
{
    opts = ThresholdSieveOptions::Deserialize(attr.GetParam());
}

const NumberT *GetDequeueCount(CPN::QueueReaderAdapter<NumberT> &in, unsigned &incount, NumberT **buffer) {
    if (buffer) {
        if (in.Dequeue(*buffer, incount)) {
            return *buffer;
        } else {
            incount = in.Count();
            if (incount != 0) {
                if (in.Dequeue(*buffer, incount)) {
                    return *buffer;
                }
            }
        }
    } else {
        const NumberT *inbuff = in.GetDequeuePtr(incount);
        if (!inbuff) {
            incount = in.Count();
            if (incount != 0) {
                inbuff = in.GetDequeuePtr(incount);
            }
        }
        return inbuff;
    }
    return 0;
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
    REPORT(oss.str().c_str());
}

void ThresholdSieveFilter::Process() {
    DEBUG("%s started\n", GetName().c_str());
    CPN::QueueReaderAdapter<NumberT> in = GetReader(IN_PORT);
    CPN::QueueWriterAdapter<NumberT> out = GetWriter(CONTROL_PORT);
    const bool zerocopy = opts.zerocopy;
    const unsigned long threshold = opts.threshold;
    const NumberT cutoff = (NumberT)(ceil(sqrt(opts.maxprime)));
    unsigned long long tot_processed = 0;
    unsigned long long tot_passed = 0;
    NumberT buffer[threshold];
    NumberT buffer2[threshold];
    NumberT buffer3[threshold];
    NumberT *buffer3_ptr1 = buffer3;
    NumberT **buffer3_ptr = &buffer3_ptr1;
    NumberT *outbuff = 0;
    if (zerocopy & WRITE_COPY) {
        outbuff = buffer2;
    }
    const NumberT ppf = PrimesPerFilter();
    PrimeSieve sieve(ppf);
    unsigned numPrimes = 0;
    unsigned numPassed = 0;
    unsigned incount = threshold;
    bool loop = true;
    while (loop && (numPassed == 0) ) {
        incount = threshold;
        const NumberT *inbuff = GetDequeueCount(in, incount, (zerocopy & READ_COPY ? buffer3_ptr : 0));
        if (!inbuff) {
            loop = false;
        } else {
            tot_processed += incount;
            if (!(zerocopy & WRITE_COPY)) {
                outbuff = out.GetEnqueuePtr(incount);
            }
            sieve.TryCandidates(inbuff, incount, outbuff, numPrimes, buffer, numPassed);
#if _DEBUG
            ReportCandidates(inbuff, incount, outbuff, numPrimes, buffer, numPassed);
#endif
            tot_passed += numPrimes;
            if (zerocopy & WRITE_COPY) {
                out.Enqueue(outbuff, numPrimes);
            } else {
                out.Enqueue(numPrimes);
            }
            if (!(zerocopy & READ_COPY)) {
                in.Dequeue(incount);
            }
            REPORT("%s processed primes %u -> %u (%u)\n", GetName().c_str(), incount, numPrimes, numPassed);
        }
    }
    if (loop) {
        if (buffer[0] <= cutoff) {
            out.Release();
            CreateNewFilter();
            DEBUG("%s created new filter\n", GetName().c_str());
            out = GetWriter(OUT_PORT);
        }
        tot_passed += numPassed;
        out.Enqueue(buffer, numPassed);
    }
    while (loop) {
        incount = threshold;
        const NumberT *inbuff = GetDequeueCount(in, incount, (zerocopy & READ_COPY ? buffer3_ptr : 0));
        if (!inbuff) {
            loop = false;
        } else {
            if (!(zerocopy & WRITE_COPY)) {
                outbuff = out.GetEnqueuePtr(incount);
            }
            sieve.TryCandidates(inbuff, incount, buffer, numPrimes, outbuff, numPassed);
#if _DEBUG
            ReportCandidates(inbuff, incount, buffer, numPrimes, outbuff, numPassed);
#endif
            ASSERT(numPrimes == 0);
            if (zerocopy & WRITE_COPY) {
                out.Enqueue(outbuff, numPassed);
            } else {
                out.Enqueue(numPassed);
            }
            if (!(zerocopy & READ_COPY)) {
                in.Dequeue(incount);
            }
            tot_processed += incount;
            tot_passed += numPassed;
            REPORT("%s processed candidates %u -> %u (%u)\n", GetName().c_str(), incount, numPassed, numPrimes);
        }
    }
    out.Release();
    in.Release();
    if (opts.report) {
        printf("%s statistics: PPF: %llu\n\tProcessed:\t%llu\n\tPassed:  \t%llu\n\tStopped:\t%llu\n",
            GetName().c_str(), ppf, tot_processed, tot_passed, tot_processed - tot_passed);
    }
    DEBUG("%s stopped\n", GetName().c_str());
}

NumberT ThresholdSieveFilter::PrimesPerFilter() {
    double filter = opts.filtercount;
    double power = 1;
    double ppf = 0;
    std::vector<double>::iterator itr = opts.primesPerFilter.begin();
    while (opts.primesPerFilter.end() != itr) {
        ppf += *itr * power;
        power *= filter;
        ++itr;
    }
    ppf = floor(ppf);
    if (ppf < 1) { ppf = 1; }
    DEBUG("%s filtering primes %f\n", GetName().c_str(), ppf);
    return (NumberT)ppf;
}


