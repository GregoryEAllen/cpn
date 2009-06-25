/** \file
 * The QueueFactory for ThresholdQueue
 */

#ifndef CPN_THRESHOLDQUEUEFACTORY_H
#define CPN_THRESHOLDQUEUEFACTORY_H

#include "common.h"
#include "QueueFactory.h"

namespace CPN {
	class ThresholdQueueFactory : public CPN::QueueFactory {
	public:
		~ThresholdQueueFactory();

		CPN::QueueBase* Create(const CPN::QueueAttr &attr);

		void Destroy(CPN::QueueBase* queue);

		static ThresholdQueueFactory* GetInstance() { return &instance; }
	private:
		ThresholdQueueFactory();
		static ThresholdQueueFactory instance;
	};

}

#endif
