
#ifndef PRODUCERNODE_H
#define PRODUCERNODE_H

#include "NodeBase.h"
#include "Kernel.h"

class ProducerNode : public CPN::NodeBase {
public:
	ProducerNode(CPN::Kernel &ker, const CPN::NodeAttr &attr) :
	       NodeBase(ker, attr, inputs, 0, outputs, 1) {}
	void Process(void);
private:
	static const ::std::string inputs[];
	static const ::std::string outputs[];
};

#endif
