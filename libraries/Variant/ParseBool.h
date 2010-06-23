
#pragma once
#include <string>
/**
 * \brief Parses string for a boolean value.
 *
 * Considers no, false, 0 and any abreviation
 * like "   f" to be false
 * and everything else to be true (including
 * the empty string).
 * Ignores whitespace.
 * Note "false blah" is true!
 * \return true or false
 * \param str the string to parse
 */
bool ParseBool(const std::string &str);
