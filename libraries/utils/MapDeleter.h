/** \file
 * This functor class is ment to be
 * passed to a for_each.
 */
#ifndef MAPDELETER_H
#define MAPDELETER_H
#include <utility>
/**
 * A simple functor to be passed to 
 * stl algorithms which assumes the second
 * parameter in the pair passed in is a pointer
 * that needs to be deleted.
 */
template<class thekey, class thetype>
class MapDeleter {
public:
	void operator() (std::pair<thekey, thetype*> o) {
		if (o.second) {
			delete o.second;
		}
	}
};
#endif

