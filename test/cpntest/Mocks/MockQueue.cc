
#include "MockQueue.h"
#include "MockQueueFactory.h"
#include <cstdio>

static MockQueueFactory theFactory("MockQueue");

// From QueueWriter
void* MockQueue::GetRawEnqueuePtr(CPN::ulong thresh, CPN::ulong chan) {
	return 0;
}
void MockQueue::Enqueue(CPN::ulong count) {
}
bool MockQueue::RawEnqueue(void* data, CPN::ulong count, CPN::ulong chan) {
	return false;
}
ulong MockQueue::NumChannels(void) const {
	return 0;
}
ulong MockQueue::Freespace(void) const {
	return 0;
}
bool MockQueue::Full(void) const {
	return true;
}

// From QueueReader
const void* MockQueue::GetRawDequeuePtr(CPN::ulong thresh, CPN::ulong chan) {
	return 0;
}
void MockQueue::Dequeue(ulong count) {
}
bool MockQueue::RawDequeue(void * data, CPN::ulong count, CPN::ulong chan) {
	return false;
}
//ulong NumChannels(void) const;
CPN::ulong MockQueue::Count(void) const {
	return 0;
}
bool MockQueue::Empty(void) const {
	return false;
}

// From QueueBase
CPN::ulong MockQueue::ElementsEnqueued(void) const {
	return 0;
}
CPN::ulong MockQueue::ElementsDequeued(void) const {
	return 0;
}

void MockQueue::RegisterReaderEvent(PthreadCondition* evt) {
}

void MockQueue::RegisterWriterEvent(PthreadCondition* evt) {
}

