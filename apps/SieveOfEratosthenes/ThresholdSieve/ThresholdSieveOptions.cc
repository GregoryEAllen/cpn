
#include "ThresholdSieveOptions.h"
#include "ThresholdSieveFilter.h"
#include "Kernel.h"
#include "ToString.h"


void CreateNewFilter(CPN::Kernel &kernel, ThresholdSieveOptions &opts, CPN::Key_t ourkey) {
    ++opts.filtercount;
    std::string nodename = ToString(FILTER_FORMAT, opts.filtercount);
    CPN::NodeAttr attr (nodename, THRESHOLDSIEVEFILTER_TYPENAME);
    attr.SetParam(StaticBuffer(&opts, sizeof(opts)));
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

