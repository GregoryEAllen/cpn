
#ifndef MOCKNODE_H
#define MOCKNODE_H

#include "NodeBase.h"
/**
 * A mock node for testing general testing.
 * Takes a mode to define it's behavior.
 */
class MockNode : public CPN::NodeBase {
public:
	enum Mode_t { MODE_SOURCE = 1, MODE_TRANSMUTE, MODE_SINK, MODE_NOP };

	MockNode(CPN::Kernel &ker, const CPN::NodeAttr &attr, const Mode_t &mode_) 
		: CPN::NodeBase(ker, attr), mode(mode_) {}
	MockNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
		: CPN::NodeBase(ker, attr), mode(MODE_NOP) {}
	MockNode(CPN::Kernel &ker, const CPN::NodeAttr &attr, const std::string &param)
		: CPN::NodeBase(ker, attr), mode(GetMode(param)) {
	}

	void Process(void);

private:
	Mode_t GetMode(const std::string &param) {
		if (param == "source") return MODE_SOURCE;
		else if (param == "transmute") return MODE_TRANSMUTE;
		else if (param == "sink") return MODE_SINK;
		else return MODE_NOP;
	}
	const Mode_t mode;
};


#endif
