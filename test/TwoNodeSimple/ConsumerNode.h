
#ifndef CONSUMERNODE_H
#define CONSUMERNODE_H

#include "NodeBase.h"
#include "Kernel.h"

class ConsumerNode : public CPN::NodeBase {
public:
	ConsumerNode(CPN::Kernel &ker, const CPN::NodeAttr &attr) :
	       	NodeBase(ker, attr, inputs, 1, outputs, 0) {}
	void Process(void);
private:
	static const ::std::string inputs[];
	static const ::std::string outputs[];
};

#endif
