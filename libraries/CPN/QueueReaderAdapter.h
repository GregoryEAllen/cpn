/** \file
 * \brief Implementation and definition for QueueReader adapter.
 */

#ifndef CPN_QUEUE_QUEUEREADERADAPTER_H
#define CPN_QUEUE_QUEUEREADERADAPTER_H

#include "QueueReader.h"

namespace CPN {
	/** 
	 * Template class to do type conversion for reader end of the queue.
	 */
	template<class T>
	class QueueReaderAdapter {
	public:
		QueueReaderAdapter(QueueReader* q) : rqueue(q) {}

		QueueReader* GetReader(void) { return rqueue; }

		const T* GetDequeuePtr(CPN::ulong thresh, CPN::ulong chan=0) {
			return (T*) rqueue->GetRawDequeuePtr(sizeof(T) * thresh, chan);
		}

		void Dequeue(CPN::ulong count) {
			rqueue->Dequeue(sizeof(T) * count);
		}

		bool Dequeue(T* data, CPN::ulong count, CPN::ulong chan=0) {
			return rqueue->RawDequeue((void*)data, sizeof(T) * count, chan);
		}

	private:
		QueueReader* rqueue;
	};
}
#endif
