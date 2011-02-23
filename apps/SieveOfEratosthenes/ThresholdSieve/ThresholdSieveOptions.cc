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
#include "ThresholdSieveOptions.h"
#include "ThresholdSieveFilter.h"
#include "Kernel.h"
#include "ToString.h"
#include "Variant.h"
#include "VariantToJSON.h"
#include "JSONToVariant.h"
#include "Assert.h"

#include <stdio.h>

void CreateNewFilter(CPN::Kernel &kernel, ThresholdSieveOptions &opts, CPN::Key_t ourkey) {
    ++opts.filtercount;
    std::string nodename = ToString(FILTER_FORMAT, opts.filtercount);
    CPN::NodeAttr attr (nodename, THRESHOLDSIEVEFILTER_TYPENAME);
    if (!opts.kernels.empty()) {
        int num = opts.filtercount;
        if (opts.divisor > 0) num /= opts.divisor;
        num %= opts.kernels.size();
        attr.SetHost(opts.kernels[num]);
    }
    if (opts.report) {
        fprintf(stderr, "Creating filter %llu on kernel %s\n", opts.filtercount, attr.GetHost().c_str());
    }
    attr.SetParam("options", opts.Serialize());
    CPN::Key_t nodekey = kernel.CreateNode(attr);

    CPN::QueueAttr qattr(opts.queuesize * sizeof(ThresholdSieveOptions::NumberT),
            opts.threshold * sizeof(ThresholdSieveOptions::NumberT));
    qattr.SetHint(opts.queuehint).SetDatatype<ThresholdSieveOptions::NumberT>();
    qattr.SetWriter(ourkey, OUT_PORT);
    qattr.SetReader(nodekey, IN_PORT);
    kernel.CreateQueue(qattr);

    qattr.SetWriter(nodekey, CONTROL_PORT);
    qattr.SetReader(opts.consumerkey, ToString(FILTER_FORMAT, opts.filtercount));
    kernel.CreateQueue(qattr);
}

std::string ThresholdSieveOptions::Serialize() {
    Variant v = Variant::ObjectType;
    v["maxprime"] = maxprime;
    v["filtercount"] = filtercount;
    v["queuesize"] = queuesize;
    v["threshold"] = threshold;
    v["ppf"] = Variant::ArrayType;
    std::vector<double>::iterator ppf_itr, ppf_end;
    ppf_itr = primesPerFilter.begin();
    ppf_end = primesPerFilter.end();
    for (;ppf_itr != ppf_end; ++ppf_itr) {
        v["ppf"].Append(*ppf_itr);
    }
    v["primewheel"] = numPrimesSource;
    v["queuehint"] = queuehint;
    v["outputport"] = outputport;
    v["printprimes"] = printprimes;
    v["consumerkey"] = consumerkey;
    v["report"] = report;
    v["zerocopy"] = zerocopy;
    v["kernels"] = Variant::ArrayType;
    for (std::vector<std::string>::iterator i = kernels.begin(),
            e = kernels.end(); i != e; ++i) {
        v["kernels"].Append(*i);
    }
    v["divisor"] = divisor;
    return VariantToJSON(v);
}

ThresholdSieveOptions ThresholdSieveOptions::Deserialize(const std::string &str) {
    JSONToVariant p;
    p.Parse(str);
    if (!p.Done()) {
        fprintf(stderr, "Error parsing options, line %u column %u\n", p.GetLine(), p.GetColumn());
        ASSERT(false);
    }
    Variant v = p.Get();
    ThresholdSieveOptions opts;
    opts.maxprime = v["maxprime"].AsNumber<NumberT>();
    opts.filtercount = v["filtercount"].AsNumber<NumberT>();
    opts.queuesize = v["queuesize"].AsNumber<unsigned long>();
    opts.threshold = v["threshold"].AsNumber<unsigned long>();
    for (Variant::ListIterator i = v["ppf"].ListBegin(), e = v["ppf"].ListEnd();
            i != e; ++i)
    {
        opts.primesPerFilter.push_back(i->AsNumber<double>());
    }
    opts.numPrimesSource = v["primewheel"].AsNumber<unsigned long>();
    if (v["queuehint"].IsString()) {
        if (v["queuehint"].AsString() == "threshold") {
            opts.queuehint = CPN::QUEUEHINT_THRESHOLD;
        } else {
            opts.queuehint = CPN::QUEUEHINT_DEFAULT;
        }
    } else {
        opts.queuehint = v["queuehint"].AsNumber<CPN::QueueHint_t>();
    }
    if (v["outputport"].IsString()) {
        opts.outputport = v["outputport"].AsString();
    }
    opts.printprimes = v["printprimes"].AsBool();
    opts.consumerkey = v["consumerkey"].AsNumber<CPN::Key_t>();
    opts.report = v["report"].AsBool();
    opts.zerocopy = v["zerocopy"].AsInt();
    for (Variant::ListIterator i = v["kernels"].ListBegin(),
            e = v["kernels"].ListEnd();
            i != e;
            ++i)
    {
        opts.kernels.push_back(i->AsString());
    }
    opts.divisor = v["divisor"].AsInt();
    return opts;
}
