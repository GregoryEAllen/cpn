/** \file
 * Definition for node attributes.
 */
#ifndef CPN_NODEATTR_H
#define CPN_NODEATTR_H

#include "Attribute.h"

namespace CPN {

	/**
	 * \brief Attributes for a node.
	 * \node Implementation: All members are const.
	 */
	class NodeAttr : public Attribute {
	public:
		NodeAttr(const ulong ID, const ::std::string &name) : Attribute(ID, name) {}
	private:
	};
}
#endif
