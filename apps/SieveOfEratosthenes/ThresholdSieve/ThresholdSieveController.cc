/** \file
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

class ControllerFactory : public CPN::NodeFactory {
public:
    ControllerFactory() : CPN::NodeFactory(THRESHOLDSIEVECONTROLLER_TYPENAME) {}
    shared_ptr<CPN::NodeBase> Create(CPN::Kernel &ker, const CPN::NodeAttr &attr) {
        ASSERT(attr.GetArg().GetSize() == sizeof(ThresholdSieveOptions));
        ThresholdSieveOptions *opts = (ThresholdSieveOptions*)attr.GetArg().GetBuffer();
        return shared_ptr<CPN::NodeBase>(new ThresholdSieveController(ker, attr, *opts));
    }
};

void ThresholdSieveController::Process(void) {
    DEBUG("%s started\n", GetName().c_str());
    Initialize();
    NumberT filterCount = opts.filtercount;
    CPN::QueueReaderAdapter<NumberT> in = GetReader(ToString(FILTER_FORMAT, filterCount));
    std::vector<NumberT> *results = opts.results;
    const unsigned long threshold = opts.threshold;
    const NumberT cutoff = (NumberT)(ceil(sqrt(opts.maxprime)));
    // When the last received prime is greater than cutoff there are no more filters
    do {
        unsigned inCount = threshold;
        const NumberT *inBuff = in.GetDequeuePtr(inCount);
        if (!inBuff) {
            inCount = in.Count();
            if (inCount == 0) {
                if (!results->empty() && results->back() > cutoff) {
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
        results->insert(results->end(), inBuff, inBuff + inCount);
        in.Dequeue(inCount);
    } while (true);
    in.Release();
    DEBUG("%s stopped\n", GetName().c_str());
}

void ThresholdSieveController::Initialize(void) {
    opts.filtercount = 0;
    opts.consumerkey = GetKey();

    CPN::NodeAttr attr(PRODUCER_NAME, THRESHOLDSIEVEPRODUCER_TYPENAME);
    attr.SetParam(StaticBuffer(&opts, sizeof(opts)));
    CPN::Key_t prodkey = kernel.CreateNode(attr);

    CPN::QueueAttr qattr(opts.queuesize * sizeof(NumberT), opts.threshold * sizeof(NumberT));
    qattr.SetHint(opts.queuehint).SetDatatype<NumberT>();
    qattr.SetWriter(prodkey, CONTROL_PORT);
    qattr.SetReader(GetKey(), ToString(FILTER_FORMAT, opts.filtercount));
    kernel.CreateQueue(qattr);
}

extern "C" shared_ptr<CPN::NodeFactory> cpninitThresholdSieveControllerType(void);

shared_ptr<CPN::NodeFactory> cpninitThresholdSieveControllerType(void) {
    return (shared_ptr<CPN::NodeFactory>(new ControllerFactory));
}

