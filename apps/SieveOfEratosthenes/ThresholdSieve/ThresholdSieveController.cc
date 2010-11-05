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

#include "ThresholdSieveController.h"
#include "NodeFactory.h"
#include "Kernel.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "ThresholdSieveProducer.h"
#include "ThresholdSieveFilter.h"
#include "ToString.h"
#include "Assert.h"
#include <stdexcept>
#include <cmath>
#include <stdio.h>

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

CPN_DECLARE_NODE_FACTORY(ThresholdSieveController, ThresholdSieveController);

ThresholdSieveController::ThresholdSieveController(CPN::Kernel &ker, const CPN::NodeAttr &attr)
    : CPN::NodeBase(ker, attr)
{
    opts = ThresholdSieveOptions::Deserialize(attr.GetParam());
}

void ThresholdSieveController::Process(void) {
    DEBUG("%s started\n", GetName().c_str());
    Initialize();
    NumberT filterCount = opts.filtercount;
    CPN::QueueReaderAdapter<NumberT> in = GetReader(ToString(FILTER_FORMAT, filterCount));
    CPN::QueueWriterAdapter<NumberT> out;
    if (!opts.outputport.empty()) {
        out = GetWriter(opts.outputport);
    }
    bool print = opts.printprimes;
    const unsigned long threshold = opts.threshold;
    const NumberT cutoff = (NumberT)(ceil(sqrt(opts.maxprime)));
    NumberT lastprime = 0;
    // When the last received prime is greater than cutoff there are no more filters
    do {
        unsigned inCount = threshold;
        const NumberT *inBuff = in.GetDequeuePtr(inCount);
        if (!inBuff) {
            inCount = in.Count();
            if (inCount == 0) {
                if (lastprime > cutoff) {
                    break;
                }
                ++filterCount;
                in.Release();
                in = GetReader(ToString(FILTER_FORMAT, filterCount));
                REPORT("Consumer swapped input to %llu\n", filterCount);
                continue;
            } else {
                inBuff = in.GetDequeuePtr(inCount);
            }
        }
        REPORT("Consumer Reading %u values\n", inCount);
        ASSERT(inBuff);
        lastprime = inBuff[inCount - 1];
        if (print) {
            for (const NumberT *i = inBuff, *e = &inBuff[inCount]; i != e; ++i) {
                printf("%llu, ", (unsigned long long)*i);
            }
        }
        if (!opts.outputport.empty()) {
            out.Enqueue(inBuff, inCount);
        }
        in.Dequeue(inCount);
    } while (true);
    in.Release();
    DEBUG("%s stopped\n", GetName().c_str());
}

void ThresholdSieveController::Initialize(void) {
    opts.filtercount = 0;
    opts.consumerkey = GetKey();

    CPN::NodeAttr attr(PRODUCER_NAME, THRESHOLDSIEVEPRODUCER_TYPENAME);
    attr.SetParam(opts.Serialize());
    CPN::Key_t prodkey = kernel.CreateNode(attr);

    CPN::QueueAttr qattr(opts.queuesize * sizeof(NumberT), opts.threshold * sizeof(NumberT));
    qattr.SetHint(opts.queuehint).SetDatatype<NumberT>();
    qattr.SetWriter(prodkey, CONTROL_PORT);
    qattr.SetReader(GetKey(), ToString(FILTER_FORMAT, opts.filtercount));
    kernel.CreateQueue(qattr);
}

