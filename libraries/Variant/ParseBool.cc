
#include "ParseBool.h"
#include <ctype.h>


bool ParseBool(const std::string &str) {
    static const char NO_STR[] = "no";
    static const char FALSE_STR[] = "false";
    bool result = true;
    std::string::const_iterator cur = str.begin();
    const std::string::const_iterator end = str.end();
    while (cur != end && isspace(*cur)) { ++cur; }
    if (cur == end) return result;
    const char *cmpstr = 0;
    if (tolower(*cur) == NO_STR[0]) {
        cmpstr = &NO_STR[1];
    } else if (tolower(*cur) == FALSE_STR[0]) {
        cmpstr = &FALSE_STR[1];
    } else if (*cur == '0') {
        ++cur;
        while (cur != end && *cur == '0') { ++cur; }
        while (cur != end && isspace(*cur)) { ++cur; }
        if (cur == end) { result = false; }
        return result;
    }
    if (cmpstr != 0) {
        ++cur;
        int i = 0;
        while (cur != end && cmpstr[i] != '\0' && tolower(*cur) == cmpstr[i]) {
            ++cur; ++i;
        }
        while (cur != end && isspace(*cur)) { ++cur; }
        if (cur == end) { result = false; }
    }
    return result;
}
