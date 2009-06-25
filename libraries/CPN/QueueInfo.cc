/** \file
 */

#include "QueueInfo.h"
#include "QueueAttr.h"
#include "QueueFactory.h"
#include "QueueBase.h"
#include "NodeQueueReader.h"
#include "NodeQueueWriter.h"
#include <stdexcept>


CPN::QueueInfo::QueueInfo(const CPN::QueueAttr &attr)
{
	factory = CPNGetQueueFactory(attr.GetTypeName());
	if (!factory) throw ::std::invalid_argument("The queue type name must be a valid registered name.");
	queue = factory->Create(attr);
}

CPN::QueueInfo::~QueueInfo() {
	factory->Destroy(queue);
	queue = 0;
}

void CPN::QueueInfo::SetReader(NodeQueueReader* reader_) {
	queue->RegisterReaderEvent(0);
	reader = reader_;
	if (reader) queue->RegisterReaderEvent(reader->GetEvent());
}

void CPN::QueueInfo::SetWriter(NodeQueueWriter* writer_) {
	queue->RegisterWriterEvent(0);
	writer = writer_;
	if (writer) queue->RegisterWriterEvent(writer->GetEvent());
}
