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

