/** \file
 * Class abstraction for the uuid c interface.
 */

#ifndef UUID_H
#define UUID_H

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

    operator const unsigned char* () const { return uuid; }

    std::string ToString(void) const {
        char s[37]; // See man uuid_unparse for length
        uuid_unparse(uuid, s);
        return std::string(s);
    }

    static UUID Generate(void);
    private:
        uuid_t uuid;
};

inline UUID UUID::Generate(void) {
    uuid_t uu;
    uuid_generate(uu);
    UUID id(uu);
    return id;
}

inline bool operator==(const UUID& uu1, const UUID& uu2) { return 0 == uuid_compare(uu1, uu2); }
inline bool operator!=(const UUID& uu1, const UUID& uu2) { return 0 != uuid_compare(uu1, uu2); }
inline bool operator<(const UUID& uu1, const UUID& uu2) { return  0 < uuid_compare(uu1, uu2); }
inline bool operator<=(const UUID& uu1, const UUID& uu2) { return  0 <= uuid_compare(uu1, uu2); }
inline bool operator>(const UUID& uu1, const UUID& uu2) { return  0 > uuid_compare(uu1, uu2); }
inline bool operator>=(const UUID& uu1, const UUID& uu2) { return  0 >= uuid_compare(uu1, uu2); }
#endif

