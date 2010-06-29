#include "PathUtils.h"
#include <limits.h>
#include <stdlib.h>
static const char PATH_SEP = '/';

std::string RealPath(const std::string &path) {
    std::string ret;
    char *str = realpath(path.c_str(), 0);
    if (str != 0) {
        ret = str;
        free(str);
    }
    return ret;
}

std::string DirName(const std::string &path) {
    std::string ret = ".";
    if (!path.empty()) {
        std::string::const_iterator cur = path.end();
        --cur;
        while (cur != path.begin() && *cur == PATH_SEP) { --cur; }
        if (cur == path.begin()) {
            if (*cur == PATH_SEP) {
                ret = std::string(cur, cur + 1);
            }
        } else {
            while (cur != path.begin() && *cur != PATH_SEP) { --cur; }
            if (cur == path.begin()) {
                if (*cur == PATH_SEP) {
                    ret = std::string(cur, cur + 1);
                }
            } else {
                while (cur != path.begin() && *cur == PATH_SEP) { --cur; }
                ret = std::string(path.begin(), cur + 1);
            }
        }
    }
    return ret;
}

std::string BaseName(const std::string &path) {
    std::string ret;
    if (!path.empty()) {
        std::string::const_iterator end = path.end();
        std::string::const_iterator start;
        --end;
        while (end != path.begin() && *end == PATH_SEP) { --end; }
        if (end == path.begin()) {
            if (*end == PATH_SEP) {
                ret = std::string(end, end + 1);
            }
        } else {
            start = end;
            while (start != path.begin() && *start != PATH_SEP) { --start; }
            if (*start == PATH_SEP) { ++start; }
            ret = std::string(start, end + 1);
        }
    }
    return ret;
}

std::string PathConcat(const std::string &dir, const std::string &file) {
    return dir + PATH_SEP + file;
}

bool IsAbsPath(const std::string &path) {
    bool ret = false;
    if (!path.empty()) {
        if (path[0] == PATH_SEP) {
            ret = true;
        }
    }
    return ret;
}

