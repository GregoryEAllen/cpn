
#ifndef MOCKQUEUE_H
#define MOCKQUEUE_H

#include "QueueBase.h"

namespace CPN {
	class QueueFactory;
}

class MockQueue : public CPN::QueueBase {
public:
	MockQueue(const CPN::QueueAttr &qattr) : QueueBase(qattr) {}
	~MockQueue() {}

	// From QueueWriter
	void* GetRawEnqueuePtr(ulong thresh, ulong chan=0);
	void Enqueue(ulong count);
	bool RawEnqueue(void* data, ulong count, ulong chan=0);
	ulong NumChannels(void) const;
	ulong Freespace(void) const;
	bool Full(void) const;

	// From QueueReader
	const void* GetRawDequeuePtr(ulong thresh, ulong chan=0);
	void Dequeue(ulong count);
	bool RawDequeue(void * data, ulong count, ulong chan=0);
	//ulong NumChannels(void) const;
	ulong Count(void) const;
	bool Empty(void) const;

	// From QueueBase
	ulong ElementsEnqueued(void) const;
	ulong ElementsDequeued(void) const;

	void RegisterReaderEvent(PthreadCondition* evt);

	void RegisterWriterEvent(PthreadCondition* evt);
};

#endif
