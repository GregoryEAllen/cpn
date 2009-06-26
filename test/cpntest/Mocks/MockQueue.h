
#ifndef MOCKQUEUE_H
#define MOCKQUEUE_H

#include "QueueBase.h"

class MockQueue : public CPN::QueueBase {
public:
	MockQueue(const CPN::QueueAttr &qattr) : QueueBase(qattr) {}
	~MockQueue() {}

	// From QueueWriter
	void* GetRawEnqueuePtr(CPN::ulong thresh, CPN::ulong chan=0);
	void Enqueue(CPN::ulong count);
	bool RawEnqueue(void* data, CPN::ulong count, CPN::ulong chan=0);
	CPN::ulong NumChannels(void) const;
	CPN::ulong Freespace(void) const;
	bool Full(void) const;

	// From QueueReader
	const void* GetRawDequeuePtr(CPN::ulong thresh, CPN::ulong chan=0);
	void Dequeue(CPN::ulong count);
	bool RawDequeue(void * data, CPN::ulong count, CPN::ulong chan=0);
	//ulong NumChannels(void) const;
	CPN::ulong Count(void) const;
	bool Empty(void) const;

	// From QueueBase
	CPN::ulong ElementsEnqueued(void) const;
	CPN::ulong ElementsDequeued(void) const;

	void RegisterReaderEvent(PthreadCondition* evt);

	void RegisterWriterEvent(PthreadCondition* evt);
};

#endif
