//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \brief A template class used to invoke the
 * same member function on a map of classes.
 * \author John Bridgman
 */
#ifndef MAPINVOKE_H
#define MAPINVOKE_H
#pragma once
#include <utility>

/**
 * \brief Functor to invoke a class method.
 * second of the pair must act like a pointer.
 */
template<typename membertype>
class MapInvokeImpl0 {
public:
	MapInvokeImpl0(membertype memfunc) : memberfunc(memfunc) {}

    template<typename keytype, typename valuetype>
	void operator() (std::pair<keytype, valuetype> o) {
		if (o.second) {
			((*o.second).*memberfunc)();
		}
	}

private:
	membertype memberfunc;
};
/**
 * \brief version of MapInvoke that passes a paramater
 */
template<typename membertype, typename T>
class MapInvokeImpl1 {
public:
	MapInvokeImpl1(membertype memfunc, T &p)
        : memberfunc(memfunc), param(p) {}

    template<typename keytype, typename valuetype>
	void operator() (std::pair<keytype, valuetype> o) {
		if (o.second) {
			((*o.second).*memberfunc)(param);
		}
	}

private:
	membertype memberfunc;
    // allow default copying of MapInvoke to not copy param
    T &param;
};

template<typename membertype>
MapInvokeImpl0<membertype> MapInvoke(membertype memfunc) {
    return MapInvokeImpl0<membertype>(memfunc);
}
template<typename membertype, typename T>
MapInvokeImpl1<membertype, T> MapInvoke(membertype memfunc, T &p) {
    return MapInvokeImpl1<membertype, T>(memfunc, p);
}

#endif

