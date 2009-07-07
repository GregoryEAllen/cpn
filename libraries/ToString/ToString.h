/** \file
 */
#ifndef TOSTRING_H
#define TOSTRING_H

#include <string>

/**
 * Takes arguments like printf and returns a std::string.
 * \param fmt format string like printf
 * \return an std::string result
 */
std::string ToString(const char *fmt, ...);

#endif

