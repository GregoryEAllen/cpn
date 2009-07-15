/** \file
 */

#include "QueueInfo.h"
#include "QueueAttr.h"
#include "QueueFactory.h"
#include "QueueBase.h"
#include "NodeQueueReader.h"
#include "NodeQueueWriter.h"
#include "NodeInfo.h"
#include "Kernel.h"
#include <stdexcept>
#include <cassert>

#if _DEBUG
#include <cstdio>
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif



CPN::QueueInfo::QueueInfo(Kernel *ker, const CPN::QueueAttr &attr)
	: kernel(ker), factory(0), queue(0), reader(0), writer(0),
	readerset(false), writerset(false)
{
	factory = CPNGetQueueFactory(attr.GetTypeName());
	if (!factory) throw ::std::invalid_argument("The queue type name must be a valid registered name.");
	queue = factory->Create(attr);
}

CPN::QueueInfo::~QueueInfo() {
	assert(0 == writer);
	assert(0 == reader);
	DEBUG("%s destroyed\n", queue->GetName().c_str());
	factory->Destroy(queue);
	queue = 0;
}

void CPN::QueueInfo::SetReader(NodeQueueReader* reader_) {
	assert(reader_);
	if (reader == reader_) return;
	ClearReader(false);
	reader = reader_;
	DEBUG("Reader %s.%s registered to %s\n", 
			reader->GetNodeInfo()->GetName().c_str(),
			reader->GetPortName().c_str(),
			queue->GetName().c_str());
	queue->SetReaderStatusHandler(reader->GetStatusHandler());
	reader->SetQueueInfo(this);
	readerset = true;
}

void CPN::QueueInfo::ClearReader(bool checkdeath) {
	NodeQueueReader* oldReader = reader;
	reader = 0;
	if (oldReader) {
		DEBUG("Reader unregistered to %s\n", 
				queue->GetName().c_str());
		queue->ClearReaderStatusHandler();
		oldReader->ClearQueueInfo(checkdeath);
	}
	if (checkdeath) CheckDeath();
}

void CPN::QueueInfo::SetWriter(NodeQueueWriter* writer_) {
	assert(writer_);
	if (writer == writer_) return;
	ClearWriter(false);
	writer = writer_;
	DEBUG("Writer %s.%s registered to %s\n", 
			writer->GetNodeInfo()->GetName().c_str(),
			writer->GetPortName().c_str(),
			queue->GetName().c_str());
	queue->SetWriterStatusHandler(writer->GetStatusHandler());
	writer->SetQueueInfo(this);
	writerset = true;
}

void CPN::QueueInfo::ClearWriter(bool checkdeath) {
	NodeQueueWriter* oldWriter = writer;
	writer = 0;
	if (oldWriter) {
		DEBUG("Writer unregistered to %s\n", 
				queue->GetName().c_str());
		queue->ClearWriterStatusHandler();
		oldWriter->ClearQueueInfo(checkdeath);
	}
	if (checkdeath) CheckDeath();
}

void CPN::QueueInfo::CheckDeath(void) {
	// We die iff we have no reader and have no writer
	// and we have had a reader and writer in the past
	if ( (0 == reader) && (0 == writer) &&
			readerset && writerset ) {
		kernel->QueueShutdown(queue->GetName());
		DEBUG("%s has died\n", queue->GetName().c_str());
	}
}

