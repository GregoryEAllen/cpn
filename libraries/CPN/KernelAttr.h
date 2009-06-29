/** \file
 * \brief Definition of the kernel attributes.
 */

#ifndef CPN_KERNELATTR_H
#define CPN_KERNELATTR_H

#include "Attribute.h"

namespace CPN {
	/**
	 * \brief The attribute for the Kernel.
	 */
	class KernelAttr : public Attribute {
	public:
		KernelAttr(const ulong id, const ::std::string &name)
			: Attribute(id, name) {}
	};
}
#endif
