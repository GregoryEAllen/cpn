/** \file
 * \brief Implementation and definition for QueueReader adaptor.
 */

#ifndef CPN_QUEUE_QUEUEREADERADAPTOR_H
#define CPN_QUEUE_QUEUEREADERADAPTOR_H

#include "QueueReader.h"

namespace CPN {
	template<class T>
	class QueueReaderAdaptor {
	public:
		QueueReaderAdaptor(QueueReader* q) : rqueue(q) {}

		const T* GetDequeuePtr(ulong thresh, ulong chan=0) {
			return (T*) rqueue->GetRawDequeuePtr(sizeof(T) * thresh, chan);
		}

		void Dequeue(ulong count) {
			rqueue->Dequeue(sizeof(T) * count);
		}

		void Dequeue(T* data, ulong count, ulong chan=0) {
			const T* src = GetDequeuePtr(count, chan);
			memcpy(data, src, sizeof(T) * count);
			Dequeue(count);
		}

	private:
		QueueReader* rqueue;
	};
}
#endif
