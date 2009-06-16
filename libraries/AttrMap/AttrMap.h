/** \file
 * \brief A template attribute map that maps attributes to
 * some type.
 */

#ifndef ATTRMAP_H
#define ATTRMAP_H
#include "Attribute.h"
#include <map>
#include <string>
/**
 * This template class provides a mapping
 * between Attributes and T. There are provided
 * functions to look up T by ether name or id.
 */
template<class T>
class AttrMap {
	typedef unsigned long ulong;

	void Insert(Attribute attr, T t) {
		idmap[attr.GetID()] = t;
		namemap[attr.GetName()] = attr.GetID();
	}

	T Get(ulong id) {
		return idmap[id];
	}

	T Get(std::string name) {
		std::map::iterator it = namemap.find(name);
		if (it == namemap.end()) {
			return T();
		}
		return Get(*it);
	}

	void Clear(void) {
		namemap.clear();
		idmap.clear();
	}
private:

	std::map<ulong, T> idmap;
	std::map<std::string, ulong> namemap;
};
#endif
