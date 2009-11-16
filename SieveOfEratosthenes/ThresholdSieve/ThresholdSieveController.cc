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
    NumberT filterCount = 1;
    Initialize();
    CPN::QueueReaderAdapter<NumberT> in = GetReader(ToString(FILTER_FORMAT, filterCount));
    std::vector<NumberT> *results = opts.results;
    const unsigned long threshold = opts.threshold;
    const NumberT cutoff = (NumberT)(ceil(sqrt(opts.maxprime)));
    NumberT primecount = 0;
    // Count the number of primes under cutoff/primesperfilter and that is how many filters there are
    // Alternatively, when the last received prime is greater than cutoff there are no more
    // filters
    do {
        unsigned inCount = threshold;
        const NumberT *inBuff = in.GetDequeuePtr(inCount);
        if (!inBuff) {
            inCount = in.Count();
            if (inCount == 0) {
                if (results->back() > cutoff) {
                    break;
                }
                ++filterCount;
                in.Release();
                in = GetReader(ToString(FILTER_FORMAT, filterCount));
                continue;
            } else {
                inBuff = in.GetDequeuePtr(inCount);
            }
        }
        DEBUG("Consumer Reading %lu values\n", inCount);
        ASSERT(inBuff);
        primecount += inCount;
        results->insert(results->end(), inBuff, inBuff + inCount);
        in.Dequeue(inCount);
    } while (true);
    in.Release();
    DEBUG("%s stopped\n", GetName().c_str());
}

void ThresholdSieveController::Initialize(void) {
    ThresholdSieveProducer::RegisterNodeType();
    ThresholdSieveFilter::RegisterNodeType();
    opts.filtercount = 1;
    opts.consumerkey = GetKey();

    std::string nodename = ToString(FILTER_FORMAT, 1);
    CPN::NodeAttr attr(nodename, THRESHOLDSIEVEFILTER_TYPENAME);
    attr.SetParam(StaticBuffer(&opts, sizeof(opts)));
    kernel.CreateNode(attr);

    attr.SetName(PRODUCER_NAME).SetTypeName(THRESHOLDSIEVEPRODUCER_TYPENAME);
    kernel.CreateNode(attr);

    CPN::QueueAttr qattr(opts.queuesize * sizeof(NumberT), opts.threshold * sizeof(NumberT));
    qattr.SetHint(opts.queuehint).SetDatatype<NumberT>();

    qattr.SetWriter(nodename, CONTROL_PORT);
    qattr.SetReader(GetName(), ToString(FILTER_FORMAT, 1));
    kernel.CreateQueue(qattr);

    qattr.SetWriter(PRODUCER_NAME, OUT_PORT);
    qattr.SetReader(nodename, IN_PORT);
    kernel.CreateQueue(qattr);
}

void ThresholdSieveController::RegisterNodeType(void) {
    CPNRegisterNodeFactory(shared_ptr<CPN::NodeFactory>(new ControllerFactory));
}

