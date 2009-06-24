/** \file
 * Definition for node attributes.
 */
#ifndef CPN_NODEATTR_H
#define CPN_NODEATTR_H

#include "Attribute.h"

namespace CPN {

	/**
	 * \brief Attributes for a node.
	 * \note Implementation: All members are const.
	 */
	class NodeAttr : public Attribute {
	public:
		NodeAttr(const ulong id_, const ::std::string &name_,
				const ::std::string &nodetype_) :
		       	Attribute(id_, name_), nodetype(nodetype_) {}

		const ::std::string &GetTypeName(void) const { return nodetype; }
	private:
		const ::std::string nodetype;
	};
}
#endif
