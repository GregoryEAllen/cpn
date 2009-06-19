/** \file
 * \brief Definition of the queue attributes.
 */

#ifndef CPN_QUEUEATTR_H
#define CPN_QUEUEATTR_H

#include "Attribute.h"

namespace CPN {
	
	class QueueAttr : public Attribute {
	public:
		QueueAttr(const ulong ID, const ::std::string &name)
			: Attribute(ID, name) {}
	private:
		const ulong queueLength;
		const ulong queueMaxThreshold;
		const ulong numChannels;
	};
}
#endif
