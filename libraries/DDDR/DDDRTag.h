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
#ifndef DDDR_TAG_H
#define DDDR_TAG_H
#pragma once
/** 
 * \brief DDDR algorithm as described in "A Distributed Deadlock Detection And
 * Resolution Algorithm For Process Networks" by Allen, Zucknick and Evans
 */
namespace DDDR {

    class Tag {
    public:
        /**
         * Perform comparison as specified in "A Distributed Deadlock Detection And
         * Resolution Algorithm For Process Networks" by Allen, Zucknick and Evans
         */
        bool operator<(const Tag &t) const;
        bool operator>=(const Tag &t) const { return !(*this < t); }
        bool operator<=(const Tag &t) const { return !(t < *this); }
        bool operator>(const Tag &t) const { return (t < *this); }
        bool operator==(const Tag &t) const;
        bool operator!=(const Tag &t) const { return !(*this == t); }

        void Reset();

        unsigned Count() const { return count; }
        unsigned Count(unsigned c) { return count = c; }
        unsigned long long Key() const { return key; }
        unsigned long long Key(unsigned long long k) { return key = k; }
        unsigned QueueSize() const { return qsize; }
        unsigned QueueSize(unsigned qs) { return qsize = qs; }
    private:
        unsigned count;
        unsigned long long key;
        unsigned qsize;
    };

}
#endif
