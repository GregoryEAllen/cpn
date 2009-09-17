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
 * \brief Class abstraction for the uuid c interface.
 * \author John Bridgman
 */

#ifndef UUID_H
#define UUID_H
#pragma once

#include <uuid/uuid.h>
#include <string>

/**
 * A convenience wrapper around a uuid.
 *
 * The uuid type is simply a 128 bit number.
 */
class UUID {
public:
    UUID() { uuid_clear(uuid); }

    UUID(const std::string &uu) {
        if (uuid_parse(uu.c_str(), uuid)) { uuid_clear(uuid); }
    }

    explicit UUID(const uuid_t uu) { uuid_copy(uuid, uu); }

    UUID &operator=(const uuid_t uu) { uuid_copy(uuid, uu); return *this; }

    const unsigned char* Get() const { return uuid; }
    unsigned char* Get() { return uuid; }

    std::string ToString(void) const {
        char s[37]; // See man uuid_unparse for length
        uuid_unparse(uuid, s);
        return std::string(s);
    }

    void Generate(void) { uuid_generate(uuid); }
    void Clear(void) { uuid_clear(uuid); }
    bool IsNull(void) { return uuid_is_null(uuid) == 1; }
    private:
        uuid_t uuid;
};

inline bool operator==(const UUID& uu1, const UUID& uu2) { return 0 == uuid_compare(uu1.Get(), uu2.Get()); }
inline bool operator!=(const UUID& uu1, const UUID& uu2) { return 0 != uuid_compare(uu1.Get(), uu2.Get()); }
inline bool operator<(const UUID& uu1, const UUID& uu2) { return  0 < uuid_compare(uu1.Get(), uu2.Get()); }
inline bool operator<=(const UUID& uu1, const UUID& uu2) { return  0 <= uuid_compare(uu1.Get(), uu2.Get()); }
inline bool operator>(const UUID& uu1, const UUID& uu2) { return  0 > uuid_compare(uu1.Get(), uu2.Get()); }
inline bool operator>=(const UUID& uu1, const UUID& uu2) { return  0 >= uuid_compare(uu1.Get(), uu2.Get()); }
#endif

