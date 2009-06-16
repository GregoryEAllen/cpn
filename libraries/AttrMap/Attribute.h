/** \file
 * Definition for generic attributes.
 */
#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H
#include <string>


/**
 * \brief Generic declarations for all attributes.
 * \node Implementation: All members are const.
 * 
 * \node All derived classes should have all const members.
 */
class Attribute {
public:
	typedef unsigned long ulong;

	Attribute(const ulong id, const ::std::string &name)
		: id(id), name(name) {}

	ulong GetID(void) const { return id; }

	const ::std::string &GetName(void) const { return name; }

private:
	/// \brief Unique id
	const ulong id;
	/// \brief Name
	const ::std::string name;
};
#endif
