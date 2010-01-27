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
#include "D4RTag.h"

namespace D4R {

    bool Tag::operator<(const Tag &t) const {
        if (count < t.count) { return true; }
        else if (count > t.count) { return false; }
        // count == t.count
        if (key < t.key) { return true; }
        else if (key > t.key) { return false; }
        // key == t.key
        if (qsize > t.qsize) { return true; }
        return false;
    }

    bool Tag::operator==(const Tag &t) const {
        return (count == t.count && key == t.key && qsize == t.qsize);
    }

    void Tag::Reset() {
        count = 0;
        qsize = -1;
    }
}
