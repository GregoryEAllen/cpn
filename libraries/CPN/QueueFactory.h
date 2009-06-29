/** \file
 * \brief Definition of the QueueFactory.
 */
#ifndef CPN_QUEUEFACTORY_H
#define CPN_QUEUEFACTORY_H

#include <string>

namespace CPN {

	class QueueBase;
	class QueueAttr;

	/**
	 * \brief QueueFactory implements an easy way for the kernel to 
	 * create user defined nodes.
	 *
	 * \see NodeFactory
	 */
	class QueueFactory {
	public:
		QueueFactory(const ::std::string &name_);

		virtual ~QueueFactory();

		/**
		 * Create a new queue with the given attributes.
		 * \param attr the attributes \see QueueAttr
		 * \return a new object of type QueueBase
		 */
		virtual QueueBase* Create(const QueueAttr &attr) = 0;

		/**
		 * Destroy a queue created by Create.
		 * \param queue the queue created with Create
		 */
		virtual void Destroy(QueueBase* queue) = 0;

		/**
		 * \return the type name for the QueueFactory.
		 */
		const ::std::string& GetName(void) const { return name; }

	private:
		const ::std::string name;
	};

}

extern "C" {
	/**
	 * Get the factory for the given queue type.
	 * \warning all factories must have static program lifetime.
	 * \param qtypename the type of the queue factory to get
	 * \return the QueueFactory for the given queue type.
	 */
	CPN::QueueFactory* CPNGetQueueFactory(const ::std::string& qtypename);

	/**
	 * Register the given factory in our registry
	 * \warning all factories must have static program lifetime.
	 * \param fact the factory to register
	 */
	void CPNRegisterQueueFactory(CPN::QueueFactory* fact);

	/**
	 * Unregister a given factory from the regisry.
	 * \warning all factories must have static program lifetime.
	 * \param qtypename the name of the factory
	 */
	void CPNUnregisterQueueFactory(const ::std::string& qtypename);
}
#endif
