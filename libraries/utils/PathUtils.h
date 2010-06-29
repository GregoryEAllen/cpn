#pragma once
#include <string>

/**
 * Fully resolve path to its absolute path.
 * path must exist. Follows symlinks.
 */
std::string RealPath(const std::string &path);

/**
 * \return the directory name of path.
 * Trailing / are not considered part of the path.
 * If no / is contained inside path then . is returned.
 *
 * This was written to provide a reentrant version of
 * dirname. Should work the same as dirname in the
 * POSIX 1003.1-2008 specification.
 * Note that giving '..' returns '.' which is what
 * the POSIX standard says but man basename does '..'
 * gives '..' on most systems.
 */
std::string DirName(const std::string &path);

/**
 * Essentially the same as basename from libgen
 * but this one is reentrant.
 */
std::string BaseName(const std::string &path);

/**
 * Convinience function to concatinate a filename to a directory
 * in a platform dependent way.
 */
std::string PathConcat(const std::string &dir, const std::string &file);

/**
 * \returns true if path is an absolute path.
 */
bool IsAbsPath(const std::string &path);
