/** \file
 * A template class used to invoke the
 * same member function on a map of classes.
 */
#ifndef MAPINVOKE_H
#define MAPINVOKE_H

/**
 * \brief Functor to invoke a class method.
 */
template<class keytype, class valuetype, class membertype>
class MapInvoke {
public:
	MapInvoke(membertype memfunc) : memberfunc(memfunc) {}

	void operator() (std::pair<keytype, valuetype*> o) {
		if (o.second) {
			((*o.second).*memberfunc)();
		}
	}

private:
	membertype memberfunc;
};
#endif

