/** \file
 * \brief Definition and implementation of an adaptor for
 * the CPN QueueWriter.
 */

#ifndef CPN_QUEUE_QUEUEWRITERADAPTOR_H
#define CPN_QUEUE_QUEUEWRITERADAPTOR_H

#include "QueueWriter.h"

namespace CPN {
	/**
	 * This template class does type conversion.
	 */
	template<class T>
	class QueueWriterAdapter {
	public:
		QueueWriterAdapter(QueueWriter* q) : queuew(q) {
		}

		QueueWriter* GetWriter(void) { return queuew; }

		T* GetEnqueuePtr(CPN::ulong thresh, CPN::ulong chan=0) {
			return (T*) queuew->GetRawEnqueuePtr(sizeof(T) * thresh, chan);
		}
		
		void Enqueue(CPN::ulong count) {
			queuew->Enqueue(sizeof(T) * count);
		}

		bool Enqueue(T* data, CPN::ulong count, CPN::ulong chan=0) {
			return queuew->RawEnqueue((void*)data, sizeof(T) * count, chan);
		}

	private:
		QueueWriter* queuew;
	};
}
#endif
