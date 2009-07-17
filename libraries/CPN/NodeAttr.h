/** \file
 * Definition for node attributes.
 */
#ifndef CPN_NODEATTR_H
#define CPN_NODEATTR_H

#include "common.h"

namespace CPN {

	/**
	 * \brief Attributes for a node.
	 *
	 * \note Implementation: All members are const.
	 */
	class CPN_API NodeAttr {
	public:
		NodeAttr(const ulong id_, const ::std::string &name_,
				const ::std::string &nodetype_) :
		       	id(id_), name(name_), nodetype(nodetype_) {}

		ulong GetID(void) const { return id; }

		const ::std::string &GetName(void) const { return name; }

		const std::string &GetTypeName(void) const { return nodetype; }
	private:
		const ulong id;
		const std::string name;
		const std::string nodetype;
	};
}
#endif
