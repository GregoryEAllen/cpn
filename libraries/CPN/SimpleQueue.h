/** \file
 * A simple implementation of the QueueBase interface.
 */

#ifndef CPN_SIMPLEQUEUE_H
#define CPN_SIMPLEQUEUE_H

#include "QueueBase.h"
#include "PthreadMutex.h"
#include "ReentrantLock.h"
#include <vector>

#define CPN_QUEUETYPE_SIMPLE "CPN::SimpleQueue"

namespace CPN {

	/**
	 * /brief A very simple implementatin of QueueBase.
	 * 
	 * No special memory mapping or any other stuff, just a
	 * plain memory buffer. Multiple calls the Enqueue are not
	 * checked for invalid input. So the user can try to 
	 * submit more than there queue space and predictable
	 * errors will follow.
	 * 
	 * \note This queue type does not support multiple channels.
	 * the number of channels on creation must be 1 and the channel
	 * parameter must always be 0.
	 */
	class SimpleQueue : public QueueBase {
	public:

		SimpleQueue(const QueueAttr& attr);
		~SimpleQueue();

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

		static void RegisterQueueType(void);
	private:
		std::vector<char> buffer;
		unsigned long queueLength;
		unsigned long maxThreshold;
		// head points to the next empty byte
		unsigned long head;
		// tail points to the next byte to read
		unsigned long tail;
		mutable Sync::ReentrantLock qlock;
		PthreadCondition* qwritten;
		PthreadCondition* qread;
	};
}

#endif

