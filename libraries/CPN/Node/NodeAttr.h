/** \file
 * Definition for node attributes.
 */
#ifndef CPN_NODEATTR_H
#define CPN_NODEATTR_H

#include "CPNAttr.h"

namespace CPN {

	/**
	 * \brief Attributes for a node.
	 * \node Implementation: All members are const.
	 */
	class NodeAttr : public CPNAttr {
	public:
		NodeAttr(const ulong ID, const ::std::string &name)
			: CPNAttr(ID, name) {}
	private:
		// List of input and output ports?
	};
}
#endif
