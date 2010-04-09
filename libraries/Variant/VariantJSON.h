
#pragma once
#include <iosfwd>
#include "Variant.h"

class PrettyJSON {
public:
    explicit PrettyJSON(const Variant &v, bool p = true) : value(v), pretty(p) {}
    const Variant &value;
    const bool pretty;
};

std::string VariantToJSON(const Variant &v, bool pretty = false);

std::ostream &VariantToJSON(std::ostream &os, const Variant &v, bool pretty);

inline std::ostream &operator<<(std::ostream &os, const Variant &v) {
    return VariantToJSON(os, v, false);
}

inline std::ostream &operator<<(std::ostream &os, const PrettyJSON &v) {
    return VariantToJSON(os, v.value, v.pretty);
}


