/** \file
 */

#include "NodeFactory.h"
#include "NodeInfo.h"
#include "NodeAttr.h"
#include "BlockingQueueWriter.h"
#include "BlockingQueueReader.h"
#include "MapInvoke.h"
#include "NodeBase.h"
#include <stdexcept>
#include <algorithm>



CPN::NodeInfo::NodeInfo(Kernel &ker, const NodeAttr &attr,
	       	const void* const arg, const ulong argsize)
	: factory(0), node(0)
{
	factory = CPNGetNodeFactory(attr.GetTypeName());
	if (!factory) throw ::std::invalid_argument("Node type name must be a valid registered type.");
	node = factory->Create(ker, attr, arg, argsize);
	node->Start();
}

CPN::NodeInfo::~NodeInfo() {
	Terminate();
	std::map<std::string, NodeQueueReader*>::iterator inputsIter = inputs.begin();
	for (;inputsIter != inputs.end(); inputsIter++) {
		(*inputsIter).second->ClearQueueInfo(true);
		delete (*inputsIter).second;
	}
	inputs.clear();
	std::map<std::string, NodeQueueWriter*>::iterator outputsIter = outputs.begin();
	for (;outputsIter != outputs.end(); outputsIter++) {
		(*outputsIter).second->ClearQueueInfo(true);
		delete (*outputsIter).second;
	}
	outputs.clear();
	factory->Destroy(node);
	node = 0;
}

void CPN::NodeInfo::Terminate(void) {
	for_each(inputs.begin(), inputs.end(), MapInvoke<std::string, CPN::NodeQueueReader,
			void (CPN::NodeQueueReader::*)(void)>(&CPN::NodeQueueReader::Terminate));
	for_each(outputs.begin(), outputs.end(),MapInvoke<std::string, CPN::NodeQueueWriter,
			void (CPN::NodeQueueWriter::*)(void)>(&CPN::NodeQueueWriter::Terminate));
}

void CPN::NodeInfo::SetWriter(QueueInfo* queue, const std::string &portname) {
	NodeQueueWriter* writer = GetWriter(portname);
	if (queue) {
		queue->SetWriter(writer);
	}
}

void CPN::NodeInfo::ClearWriter(const std::string &portname) {
	NodeQueueWriter* writer = GetWriter(portname);
	writer->ClearQueueInfo(true);
	delete writer;
	outputs.erase(portname);
}

CPN::NodeQueueWriter* CPN::NodeInfo::GetWriter(const std::string &name) {
	if (!outputs[name]) {
		outputs[name] = new CPN::BlockingQueueWriter(this, name);
	}
	return outputs[name];
}

void CPN::NodeInfo::SetReader(QueueInfo* queue, const std::string &portname) {
	NodeQueueReader* reader = GetReader(portname);
	if (queue) {
		queue->SetReader(reader);
	}
}

void CPN::NodeInfo::ClearReader(const std::string &portname) {
	NodeQueueReader* reader = GetReader(portname);
	reader->ClearQueueInfo(true);
	delete reader;
	inputs.erase(portname);
}

CPN::NodeQueueReader* CPN::NodeInfo::GetReader(const std::string &name) {
	if (!inputs[name]) {
		inputs[name] = new CPN::BlockingQueueReader(this, name);
	}
	return inputs[name];
}
