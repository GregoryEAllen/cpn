/** \file
 */

#include "QueueInfo.h"
#include "QueueAttr.h"
#include "QueueFactory.h"
#include "QueueBase.h"
#include "NodeQueueReader.h"
#include "NodeQueueWriter.h"
#include "NodeInfo.h"
#include <stdexcept>
#include <cassert>

#if 1
#include <cstdio>
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif



CPN::QueueInfo::QueueInfo(const CPN::QueueAttr &attr)
	: factory(0), queue(0), reader(0), writer(0)
{
	factory = CPNGetQueueFactory(attr.GetTypeName());
	if (!factory) throw ::std::invalid_argument("The queue type name must be a valid registered name.");
	queue = factory->Create(attr);
}

CPN::QueueInfo::~QueueInfo() {
	ClearReader();
	ClearWriter();
	factory->Destroy(queue);
	queue = 0;
}

void CPN::QueueInfo::SetReader(NodeQueueReader* reader_) {
	assert(reader_);
	if (reader == reader_) return;
	ClearReader();
	reader = reader_;
	if (reader) {
		DEBUG("Reader %s.%s registered to %s\n", 
				reader->GetNodeInfo()->GetName().c_str(),
				reader->GetPortName().c_str(),
				queue->GetName().c_str());
		queue->SetReaderStatusHandler(reader->GetStatusHandler());
		reader->SetQueueInfo(this);
	}
}

void CPN::QueueInfo::ClearReader(void) {
	NodeQueueReader* oldReader = reader;
	reader = 0;
	if (oldReader) {
		DEBUG("Reader unregistered to %s\n", 
				queue->GetName().c_str());
		queue->ClearReaderStatusHandler();
		oldReader->ClearQueueInfo();
	}
}

void CPN::QueueInfo::SetWriter(NodeQueueWriter* writer_) {
	assert(writer_);
	if (writer == writer_) return;
	ClearWriter();
	writer = writer_;
	if (writer) {
		DEBUG("Writer %s.%s registered to %s\n", 
				writer->GetNodeInfo()->GetName().c_str(),
				writer->GetPortName().c_str(),
				queue->GetName().c_str());
		queue->SetWriterStatusHandler(writer->GetStatusHandler());
		writer->SetQueueInfo(this);
	}
}

void CPN::QueueInfo::ClearWriter(void) {
	NodeQueueWriter* oldWriter = writer;
	writer = 0;
	if (oldWriter) {
		DEBUG("Writer unregistered to %s\n", 
				queue->GetName().c_str());
		queue->ClearWriterStatusHandler();
		oldWriter->ClearQueueInfo();
	}
}

