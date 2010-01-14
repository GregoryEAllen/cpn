/** \file
 */

#include "ThresholdSieveFilter.h"
#include "NodeFactory.h"
#include "Kernel.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "ToString.h"
#include "PrimeSieve.h"
#include "Assert.h"
#include <cmath>
#include <stdexcept>

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

void ThresholdSieveFilter::Process() {
    DEBUG("%s started\n", GetName().c_str());
    CPN::QueueReaderAdapter<NumberT> in = GetReader(IN_PORT);
    CPN::QueueWriterAdapter<NumberT> out = GetWriter(CONTROL_PORT);
    const unsigned long threshold = opts.threshold;
    const NumberT cutoff = (NumberT)(ceil(sqrt(opts.maxprime)));
    PrimeSieve sieve(opts.primesPerFilter);
    bool createFilter = false;
    bool filterCreated = false;
    do {
        unsigned incount = threshold;
        const NumberT* inbuff = in.GetDequeuePtr(incount);
        unsigned inidx = 0;
        unsigned outidx = 0;
        if (!inbuff) {
            incount = in.Count();
            if (incount == 0) {
                break;
            } else {
                inbuff = in.GetDequeuePtr(incount);
            }
        }
        NumberT *outbuff = out.GetEnqueuePtr(threshold);
        while ( (inidx < incount) && (createFilter == false) ) {
            NumberT inval = inbuff[inidx];
            switch (sieve.TryCandidate(inval)) {
            case -1:
//  -1 if pr passed the sieve, but the sieve is full (still possible prime)
                if (filterCreated == false && inval < cutoff) {
                    createFilter = true;
                } else {
                    outbuff[outidx] = inval;
                    ++outidx;
                    ++inidx;
                }
                break;
            case 0:
//  0 if pr was stopped by the sieve
                ++inidx;
                break;
            case 1:
//  1 if pr is prime, and has been added to the sieve
                outbuff[outidx] = inval;
                ++outidx;
                ++inidx;
                break;
            default:
                assert(false);
            }
        }
        in.Dequeue(inidx);
        out.Enqueue(outidx);
        if (createFilter) {
            createFilter = false;
            filterCreated = true;
            out.Release();
            CreateNewFilter();
            out = GetWriter(OUT_PORT);
        }
    } while (true);
    out.Release();
    in.Release();
    DEBUG("%s stopped\n", GetName().c_str());
}

void ThresholdSieveFilter::CreateNewFilter() {
    ++opts.filtercount;
    std::string nodename = ToString(FILTER_FORMAT, opts.filtercount);
    CPN::NodeAttr attr (nodename, THRESHOLDSIEVEFILTER_TYPENAME);
    attr.SetParam(StaticBuffer(&opts, sizeof(opts)));
    CPN::Key_t nodekey = kernel.CreateNode(attr);

    CPN::QueueAttr qattr(opts.queuesize * sizeof(NumberT), opts.threshold * sizeof(NumberT));
    qattr.SetHint(opts.queuehint).SetDatatype<NumberT>();
    qattr.SetWriter(GetKey(), OUT_PORT);
    qattr.SetReader(nodekey, IN_PORT);
    kernel.CreateQueue(qattr);

    qattr.SetWriter(nodekey, CONTROL_PORT);
    qattr.SetReader(opts.consumerkey, ToString(FILTER_FORMAT, opts.filtercount));
    kernel.CreateQueue(qattr);
}

void ThresholdSieveFilter::RegisterNodeType() {
    CPNRegisterNodeFactory(shared_ptr<CPN::NodeFactory>(new FilterFactory));
}

