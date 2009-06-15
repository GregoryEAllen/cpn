/** \file
 * Definition for generic CPN attributes.
 */
#ifndef CPN_CPNATTR_H
#define CPN_CPNATTR_H
#include <string>
#include "common.h"

namespace ::CPN {

	/**
	 * \brief Generic declarations for all attributes.
	 * \node Implementation: All members are const.
	 * 
	 * \node All derived classes should have all const members.
	 */
	class CPNAttr {
	public:
		CPNAttr(const ulong ID, const ::std::string &name)
			: ID(ID), name(name) {}

		ulong GetID(void) const { return ID; }

		const ::std::string &GetName(void) const { return name }

	private:
		/// \brief Unique id
		const ulong ID;
		/// \brief Name
		const ::std::string name;
	};
}
#endif
