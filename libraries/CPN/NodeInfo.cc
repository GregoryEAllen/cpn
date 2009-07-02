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
	factory->Destroy(node);
	node = 0;
	std::map<std::string, NodeQueueReader*>::iterator inputsIter = inputs.begin();
	for (;inputsIter != inputs.end(); inputsIter++) {
		QueueInfo *qinfo = (*inputsIter).second->GetQueueInfo();
		if (qinfo) {
			qinfo->SetReader(0);
		}
		delete (*inputsIter).second;
	}
	inputs.clear();
	std::map<std::string, NodeQueueWriter*>::iterator outputsIter = outputs.begin();
	for (;outputsIter != outputs.end(); outputsIter++) {
		QueueInfo *qinfo = (*outputsIter).second->GetQueueInfo();
		if (qinfo) {
			qinfo->SetWriter(0);
		}
		delete (*outputsIter).second;
	}
	outputs.clear();
}

void CPN::NodeInfo::Terminate(void) {
	for_each(inputs.begin(), inputs.end(), MapInvoke<std::string, CPN::NodeQueueReader,
			void (CPN::NodeQueueReader::*)(void)>(&CPN::NodeQueueReader::Terminate));
	for_each(outputs.begin(), outputs.end(),MapInvoke<std::string, CPN::NodeQueueWriter,
			void (CPN::NodeQueueWriter::*)(void)>(&CPN::NodeQueueWriter::Terminate));
}

void CPN::NodeInfo::SetWriter(QueueInfo* queue, std::string portname) {
	NodeQueueWriter* writer = GetWriter(portname);
	QueueInfo* oqinfo = writer->GetQueueInfo();
	if (oqinfo) {
		oqinfo->SetWriter(0);
	}
	writer->SetQueueInfo(queue);
	if (queue) {
		if (queue->GetWriter()) {
			queue->GetWriter()->SetQueueInfo(0);
		}
		queue->SetWriter(writer);
	}
}

CPN::NodeQueueWriter* CPN::NodeInfo::GetWriter(std::string name) {
	if (!outputs[name]) {
		outputs[name] = new CPN::BlockingQueueWriter(this, name);
	}
	return outputs[name];
}

void CPN::NodeInfo::SetReader(QueueInfo* queue, std::string portname) {
	NodeQueueReader* reader = GetReader(portname);
	QueueInfo* oqinfo = reader->GetQueueInfo();
	// If a queue is registered with this reader already clear it
	if (oqinfo) {
		oqinfo->SetReader(0);
	}
	reader->SetQueueInfo(queue);
	if (queue) {
		// If the queue we are registering is already registered somewhere else
		// unregister it
		if (queue->GetReader()) {
			queue->GetReader()->SetQueueInfo(0);
		}
		queue->SetReader(reader);
	}
}

CPN::NodeQueueReader* CPN::NodeInfo::GetReader(std::string name) {
	if (!inputs[name]) {
		inputs[name] = new CPN::BlockingQueueReader(this, name);
	}
	return inputs[name];
}
