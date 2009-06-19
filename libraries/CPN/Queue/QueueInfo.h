/** \file
 * \brief Definition and implementation of the QueueInfo class.
 */
#ifndef CPN_QUEUEINFO_H
#define CPN_QUEUEINFO_H

#include "QueueFactory.h"
#include <string>

namespace CPN {

	class QueueInfo {
	public:
		QueueInfo(QueueFactory factory_, const QueueAttr &attr)
	       : factory(factory_)
		{
			queue = factory.Create(attr);
		}

		~QueueInfo() {
			factory.Destroy(queue);
			queue = 0;
		}

		const ::std::string &ReaderEnd(const ::std::string readerEnd_) {
			return readerEnd = readerEnd_;
	       	}

		const ::std::string &ReaderEnd(void) const { return readerEnd; }

		const ::std::string &WriterEnd(const ::std::string writerEnd_) {
			return writerEnd = writerEnd_;
		}

		const ::std::string &WriterEnd(void) const { return writerEnd; }

		QueueBase* GetQueue(void) { return queue; }

	private:
		QueueFactory factory;
		QueueBase* queue;
		::std::string readerEnd;
		::std::string writerEnd;
	};
}

#endif

