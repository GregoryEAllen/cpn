
#ifndef MOCKQUEUEFACTORY_H
#define MOCKQUEUEFACTORY_H

#include "QueueFactory.h"

#define QUEUETYPE_MOCKQUEUE "MockQueue"

class MockQueueFactory : public CPN::QueueFactory {
public:
	MockQueueFactory(const std::string &name) : QueueFactory(name) {}

	CPN::QueueBase* Create(const CPN::QueueAttr &attr);

	void Destroy(CPN::QueueBase* queue);

	static CPN::QueueFactory* GetInstance() { return &instance; }
private:
	static MockQueueFactory instance;
};

#endif
