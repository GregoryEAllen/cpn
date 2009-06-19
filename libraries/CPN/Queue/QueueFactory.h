/** \file
 * \brief Definition of the QueueFactory.
 */
#ifndef CPN_QUEUEFACTORY_H
#define CPN_QUEUEFACTORY_H

namespace CPN {

	class QueueBase;
	class QueueAttr;

	/**
	 * All derived classes must implement copy semantics.
	 */
	class QueueFactory {
	public:

		virtual QueueBase* Create(const QueueAttr &attr);

		virtual void Destroy(QueueBase* Queue);
	};

}

#endif
