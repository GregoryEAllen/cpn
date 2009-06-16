/** \file
 * \brief Definition of the kernel attributes.
 */

#ifndef CPN_KERNELATTR_H
#define CPN_KERNELATTR_H

#include "CPNAttr.h"

namespace CPN {
	class KernelAttr : public CPNAttr {
	public:
		KernelAttr(const ulong id, const ::std::string &name)
			: CPNAttr(id, name) {}
	};
}
#endif
