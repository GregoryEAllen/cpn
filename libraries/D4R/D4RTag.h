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
#ifndef D4R_TAG_H
#define D4R_TAG_H
#pragma once
/** 
 * \brief D4R algorithm as described in "A Distributed Deadlock Detection And
 * Resolution Algorithm For Process Networks" by Allen, Zucknick and Evans
 */
namespace D4R {

    typedef unsigned long long Count_t;
    typedef unsigned long long Key_t;

    class Tag {
    public:
        Tag(Key_t k) : count(0), key(k), qsize(-1) {}
        Tag() : count(0), key(0), qsize(-1) {}
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

        Count_t Count() const { return count; }
        Count_t Count(Count_t c) { return count = c; }
        Key_t Key() const { return key; }
        Key_t Key(unsigned long long k) { return key = k; }
        unsigned QueueSize() const { return qsize; }
        unsigned QueueSize(unsigned qs) { return qsize = qs; }
    private:
        Count_t count;
        Key_t key;
        unsigned qsize;
    };

}
#endif
