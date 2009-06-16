/** \file
 * \brief Definition of the queue attributes.
 */

#ifndef CPN_QUEUEATTR_H
#define CPN_QUEUEATTR_H

#include "CPNAttr.h"

namespace CPN {
	
	class QueueAttr : public CPNAttr {
	public:
		QueueAttr(const ulong ID, const ::std::string &name)
			: CPNAttr(ID, name) {}
	};
}
#endif
