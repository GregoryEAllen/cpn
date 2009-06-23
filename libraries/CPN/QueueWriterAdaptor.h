/** \file
 * \brief Definition and implementation of an adaptor for
 * the CPN QueueWriter.
 */

#ifndef CPN_QUEUE_QUEUEWRITERADAPTOR_H
#define CPN_QUEUE_QUEUEWRITERADAPTOR_H

#include "QueueWriter.h"

namespace CPN {
	/**
	 * This template class does type convirsion and provides
	 * a convenience override method for Enqueue.
	 */
	template<class T>
	class QueueWriterAdaptor {
	public:
		QueueWriterAdaptor(QueueWriter* q) : queuew(q) {
		}

		QueueWriter* GetWriter(void) { return queuew; }

		T* GetEnqueuePtr(ulong thresh, ulong chan=0) {
			return (T*) queuew->GetRawEnqueuePtr(sizeof(T) * thresh, chan);
		}
		
		void Enqueue(ulong count) {
			queuew->Enqueue(sizeof(T) * count);
		}

		/**
		 * Get a pointer into the queue and enqueue the given
		 * data.
		 */
		void Enqueue(T* data, ulong count, ulong chan=0) {
			T* dest = GetEnqueuePtr(count, chan);
			memcpy(dest, data, sizeof(T) * count);
			Enqueue(count);
		}

	private:
		QueueWriter* queuew;
	};
}
#endif
