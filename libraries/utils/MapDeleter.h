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
 * \brief This functor class is ment to be
 * passed to a for_each.
 * \author John Bridgman
 */
#ifndef MAPDELETER_H
#define MAPDELETER_H
#pragma once
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

