/** \file
 */

#include "NodeFactory.h"
#include "NodeInfo.h"
#include "NodeAttr.h"
#include "BlockingQueueWriter.h"
#include "BlockingQueueReader.h"
#include <stdexcept>
#include <algorithm>



static void DeleteReader(std::pair<std::string, CPN::NodeQueueReader*> qr) {
	delete qr.second;
}

static void DeleteWriter(std::pair<std::string, CPN::NodeQueueWriter*> qw) {
	delete qw.second;
}



CPN::NodeInfo::NodeInfo(Kernel &ker, const NodeAttr &attr,
	       	const void* const arg, const ulong argsize)
{
	factory = CPNGetNodeFactory(attr.GetTypeName());
	if (!factory) throw ::std::invalid_argument("Node type name must be a valid registered type.");
	node = factory->Create(ker, attr, arg, argsize);
}

CPN::NodeInfo::~NodeInfo() {
	factory->Destroy(node);
	node = 0;
	for_each(inputs.begin(), inputs.end(), DeleteReader);
	inputs.clear();
	for_each(outputs.begin(), outputs.end(), DeleteWriter);
	outputs.clear();
}

void CPN::NodeInfo::SetWriter(QueueInfo* queue, std::string portname) {
	GetWriter(portname)->SetQueue(queue);
}

CPN::NodeQueueWriter* CPN::NodeInfo::GetWriter(std::string name) {
	if (!outputs[name]) {
		outputs[name] = new CPN::BlockingQueueWriter(this, name);
	}
	return outputs[name];
}

void CPN::NodeInfo::SetReader(QueueInfo* queue, std::string portname) {
	GetReader(portname)->SetQueue(queue);
}

CPN::NodeQueueReader* CPN::NodeInfo::GetReader(std::string name) {
	if (!inputs[name]) {
		inputs[name] = new CPN::BlockingQueueReader(this, name);
	}
	return inputs[name];
}
