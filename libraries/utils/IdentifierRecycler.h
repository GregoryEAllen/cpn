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
 * \author John Bridgman
 */

#ifndef IDENTIFIERRECYCLER_H
#define IDENTIFIERRECYCLER_H
#pragma once

#include <vector>
#include <algorithm>


// Type T should be some type that acts like an
// unsigned integer type. It can be a class with
// a constructor that takes 0 and the ++ and --
// and comparison operators and a copy operator.
template<class T>
class IdentifierRecycler {
public:

    IdentifierRecycler() : numalloc(0) {}

    ~IdentifierRecycler() {}

    T Alloc() {
        T id;
        if (freeids.empty()) {
            id = numalloc;
            ++numalloc;
        } else {
            id = freeids.front();
            std::pop_heap(freeids.begin(), freeids.end());
            freeids.pop_back();
        }
        return id;
    }

    bool Alloc(T id) {
        if (id < numalloc) {
            typename std::vector<T>::iterator itr = std::find(freeids.begin(), freeids.end(), id);
            if (itr != freeids.end()) {
                freeids.erase(itr);
                make_heap(freeids.begin(), freeids.end());
                return true;
            }
        } else {
            while (numalloc < id) {
                freeids.push_back(numalloc);
                push_heap(freeids.begin(), freeids.end());
                ++numalloc;
            }
            ++numalloc;
            return true;
        }
        return false;
    }

    void Free(T id) {
        freeids.push_back(id);
        push_heap(freeids.begin(), freeids.end());
        while (freeids.front() == numalloc - 1) {
            --numalloc;
            std::pop_heap(freeids.begin(), freeids.end());
            freeids.pop_back();
        }
    }
private:
    T numalloc;
    std::vector<T> freeids;
};

#endif

