
#include "MockQueue.h"
#include "MockQueueFactory.h"
#include <cstdio>


// From QueueWriter
void* MockQueue::GetRawEnqueuePtr(CPN::ulong thresh, CPN::ulong chan) {
	return 0;
}
void MockQueue::Enqueue(CPN::ulong count) {
}
bool MockQueue::RawEnqueue(void* data, CPN::ulong count) {
	return false;
}
bool MockQueue::RawEnqueue(void* data, CPN::ulong count, CPN::ulong numChans, CPN::ulong chanStride) {
	return false;
}
CPN::ulong MockQueue::NumChannels(void) const {
	return 0;
}
CPN::ulong MockQueue::Freespace(void) const {
	return 0;
}
bool MockQueue::Full(void) const {
	return true;
}

// From QueueReader
const void* MockQueue::GetRawDequeuePtr(CPN::ulong thresh, CPN::ulong chan) {
	return 0;
}
void MockQueue::Dequeue(CPN::ulong count) {
}
bool MockQueue::RawDequeue(void * data, CPN::ulong count) {
	return false;
}
bool MockQueue::RawDequeue(void * data, CPN::ulong count, CPN::ulong numChans, CPN::ulong chanStride) {
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

