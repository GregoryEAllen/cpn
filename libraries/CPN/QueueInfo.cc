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
	: factory(0), queue(0), reader(0), writer(0)
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
	reader = reader_;
	if (reader) queue->RegisterReaderEvent(reader->GetEvent());
	else queue->RegisterReaderEvent(0);
}

void CPN::QueueInfo::SetWriter(NodeQueueWriter* writer_) {
	writer = writer_;
	if (writer) queue->RegisterWriterEvent(writer->GetEvent());
	else queue->RegisterWriterEvent(0);
}
