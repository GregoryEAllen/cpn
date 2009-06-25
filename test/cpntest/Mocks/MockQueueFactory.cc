

#include "MockQueueFactory.h"
#include "MockQueue.h"


MockQueueFactory MockQueueFactory::instance("MockQueue");

CPN::QueueBase* MockQueueFactory::Create(const CPN::QueueAttr &attr) {
	return new MockQueue(attr);
}


void MockQueueFactory::Destroy(CPN::QueueBase* queue) {
	delete queue;
}

