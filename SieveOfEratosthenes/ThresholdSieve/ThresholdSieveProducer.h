/** \file
 */

#ifndef THRESHOLDSIEVEPRODUCER_H
#define THRESHOLDSIEVEPRODUCER_H

#include "NodeBase.h"
#include "ThresholdSieveOptions.h"

#define THRESHOLDSIEVEPRODUCER_TYPENAME "ThresholdSieveProducerType"

class ThresholdSieveProducer : public CPN::NodeBase {
public:
	ThresholdSieveProducer(CPN::Kernel& ker, const CPN::NodeAttr& attr,
			const ThresholdSieveOptions& opts_) :
		CPN::NodeBase(ker, attr), opts(opts_) {}
	void Process();
	static void RegisterNodeType();
private:
	ThresholdSieveOptions opts;
};

#endif

