/** \file
 */

#ifndef THRESHOLDSIEVEFILTER_H
#define THRESHOLDSIEVEFILTER_H

#include "NodeBase.h"
#include "ThresholdSieveOptions.h"

#define THRESHOLDSIEVEFILTER_TYPENAME "ThresholdSieveFilterType"

class ThresholdSieveFilter : public CPN::NodeBase {
public:
    ThresholdSieveFilter(CPN::Kernel &ker, const CPN::NodeAttr &attr,
            const ThresholdSieveOptions& opts_)
            : CPN::NodeBase(ker, attr), opts(opts_) {}
    void Process();

private:
    void CreateNewFilter() { ::CreateNewFilter(kernel, opts, GetKey()); }
    void ReportCandidates(
        const ThresholdSieveOptions::NumberT *inbuff, unsigned incount,
        ThresholdSieveOptions::NumberT *primes, unsigned numPrimes,
        ThresholdSieveOptions::NumberT *passed, unsigned numPassed);

    ThresholdSieveOptions opts;
};

#endif

