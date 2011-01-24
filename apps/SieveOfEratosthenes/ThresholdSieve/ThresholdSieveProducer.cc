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

#include "ThresholdSieveProducer.h"
#include "NodeFactory.h"
#include "Kernel.h"
#include "IQueue.h"
#include "OQueue.h"
#include "Assert.h"
#include "PrimeSieveSource.h"
#include <stdexcept>
#include <sstream>


#if _DEBUG
#include <cstdio>
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

CPN_DECLARE_NODE_FACTORY(ThresholdSieveProducer, ThresholdSieveProducer);

void PrintArray(std::ostream &os, NumberT *arr, unsigned len, const char *sep) {
    for (unsigned i = 0; i < len; ++i) {
        os << arr[i];
        if (i < len -1) {
            os << sep;
        }
    }
}

ThresholdSieveProducer::ThresholdSieveProducer(CPN::Kernel& ker, const CPN::NodeAttr& attr)
    : CPN::NodeBase(ker, attr)
{
    opts = ThresholdSieveOptions::Deserialize(attr.GetParam());
}

void ThresholdSieveProducer::Process(void) {
    DEBUG("%s started\n", GetName().c_str());
    CPN::OQueue<NumberT> out = GetWriter(CONTROL_PORT);
    const NumberT cutoff = opts.maxprime;
    const unsigned long threshold = opts.threshold;
    if (opts.numPrimesSource > 0) {
        DEBUG("%s using PrimeSieveSource\n", GetName().c_str());
        NumberT primes[opts.numPrimesSource];
        PrimeSieveSource source(opts.numPrimesSource, primes);

#if _DEBUG
        {
            std::ostringstream oss;
            oss << GetName() << " primes: ";
            PrintArray(oss, primes, opts.numPrimesSource, ", ");
            REPORT(oss.str().c_str());
        }
#endif

        out.Enqueue(primes, opts.numPrimesSource);
        out.Release();
        CreateNewFilter(kernel, opts, GetKey());
        out = GetWriter(OUT_PORT);
        unsigned roundLength = source.RoundLength();
        bool loop = true;
        while (loop) {
            NumberT *outbuff = out.GetEnqueuePtr(roundLength);
            unsigned len = source.GetNextRound(outbuff);
            if (len != 0 && outbuff[len-1] > cutoff) {
                loop = false;
            }
#if _DEBUG
        {
            std::ostringstream oss;
            oss << GetName() << " candidates: ";
            PrintArray(oss, outbuff, len, ", ");
            REPORT(oss.str().c_str());
        }
#endif
            out.Enqueue(len);
        }
    } else {
        DEBUG("%s using simple loop\n", GetName().c_str());
        out.Release();
        CreateNewFilter(kernel, opts, GetKey());
        out = GetWriter(OUT_PORT);
        NumberT counter = 2;
        while (counter < cutoff) {
            NumberT index = 0;
            NumberT *outbuff = out.GetEnqueuePtr(threshold);
            while (index < threshold && counter < cutoff) {
                outbuff[index] = counter;
                ++index;
                ++counter;
            }
            out.Enqueue(index);
        }
    }

    out.Release();
    DEBUG("%s stopped\n", GetName().c_str());
}

