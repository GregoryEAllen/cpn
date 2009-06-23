/** \file
 */

#include "QueueInfo.h"
#include "QueueAttr.h"
#include "QueueFactory.h"
#include <stdexcept>


CPN::QueueInfo::QueueInfo(const CPN::QueueAttr &attr)
{
	factory = CPN::QueueFactory::GetFactory(attr.GetTypeName());
	if (!factory) throw ::std::invalid_argument("The queue type name must be a valid registered name.");
	queue = factory->Create(attr);
}

CPN::QueueInfo::~QueueInfo() {
	factory->Destroy(queue);
	queue = 0;
}

void CPN::QueueInfo::SetReader(NodeQueueReader* reader_) {
	if (reader_->GetQueue() == this) {
		reader = reader_;
	} else {
		reader_->SetQueue(this);
	}
}

void CPN::QueueInfo::SetWriter(NodeQueueWriter* writer_) {
	if (writer_->GetQueue() == this) {
		writer = writer_;
	} else {
		writer_->SetQueue(this);
	}
}
