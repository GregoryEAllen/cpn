/** \file
 */

#ifndef THRESHOLDSIEVEFILTER_H
#define THRESHOLDSIEVEFILTER_H

#include "NodeBase.h"
#include "ThresholdSieveOptions.h"

#define THRESHOLDSIEVEFILTER_TYPENAME "ThresholdSieveTypeName"

class ThresholdSieveFilter : public CPN::NodeBase {
public:
	ThresholdSieveFilter(CPN::Kernel &ker, const CPN::NodeAttr &attr,
			const ThresholdSieveOptions& opts_)
			: CPN::NodeBase(ker, attr), opts(opts_) {}
	void Process(void);

	static void RegisterNodeType(void);
private:
	void CreateNewFilter(unsigned long lastprime);

	const ThresholdSieveOptions opts;
};

#endif

