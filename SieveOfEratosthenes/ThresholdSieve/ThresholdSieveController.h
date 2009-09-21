/** \file
 */

#ifndef THRESHOLDSIEVECONTROLLER_H
#define THRESHOLDSIEVECONTROLLER_H

#include "NodeBase.h"
#include "ThresholdSieveOptions.h"

#define THRESHOLDSIEVECONTROLLER_TYPENAME "ThresholdSieveControllerType"

class ThresholdSieveController : public CPN::NodeBase {
public:
    ThresholdSieveController(CPN::Kernel &ker, const CPN::NodeAttr &attr,
            const ThresholdSieveOptions &opts_) : CPN::NodeBase(ker, attr),
            opts(opts_) {}

    void Process(void);

    static void RegisterNodeType(void);
private:
    void Initialize(void);
    ThresholdSieveOptions opts;
};

#endif

