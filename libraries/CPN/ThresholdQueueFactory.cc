/** \file
 */

#include "ThresholdQueueFactory.h"
#include "ThresholdQueue.h"

#include <cstdio>

CPN::ThresholdQueueFactory::ThresholdQueueFactory()
       : CPN::QueueFactory(CPN_QUEUETYPE_THRESHOLD) {
}

CPN::ThresholdQueueFactory::~ThresholdQueueFactory() {
}

CPN::QueueBase* CPN::ThresholdQueueFactory::Create(const CPN::QueueAttr &attr) {
	return new CPN::ThresholdQueue(attr);
}

void CPN::ThresholdQueueFactory::Destroy(CPN::QueueBase* queue) {
	delete queue;
}

CPN::ThresholdQueueFactory CPN::ThresholdQueueFactory::instance;
