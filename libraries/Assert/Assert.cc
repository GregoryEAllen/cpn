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
 * \brief Some implementation for assert.
 * \author John Bridgman
 */

#include "Assert.h"
#include "StackTrace.h"
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdarg.h>



std::string CreateMessage(const char *exp, const char *file,
        int line, const char *func, const char *msg) {
    std::ostringstream oss;
    oss << "Assert failed in " << file << ":" << line;
    oss << " in " << func;
    oss << " : " << exp << '\n' << msg << '\n';
    oss << "\nBacktrace:\n" << GetStack(3);
    return oss.str();
}

AssertException::AssertException(const std::string &msg) throw()
    : message(msg)
{
}


AssertException::~AssertException() throw() {}

const char *AssertException::what() const throw() {
    return message.c_str();
}


bool __ASSERT(const char *exp, const char *file, int line, const char *func, bool die) {
    if (die) {
        puts(CreateMessage(exp, file, line, func, "").c_str());
        abort();
    } else {
        throw AssertException(CreateMessage(exp, file, line, func, ""));
    }
}

bool __ASSERT(const char *exp, const char *file, int line, const char *func, bool die, const std::string &msg) {
    if (die) {
        puts(CreateMessage(exp, file, line, func, "").c_str());
        abort();
    } else {
        throw AssertException(CreateMessage(exp, file, line, func, msg.c_str()));
    }
}

bool __ASSERT(const char *exp, const char *file, int line, const char *func, bool die, const char *fmt, ...) {
    std::vector<char> buff(128, '\0');
    // Note this is based on the exmaple in the man page for vsnprintf.
    while (1) {
        /* Try to print in the allocated space. */
        va_list ap;
        va_start(ap, fmt);
        int n = vsnprintf(&buff[0], buff.size(), fmt, ap);
        va_end(ap);
        /* If that worked, return the string. */
        if (n > -1 && unsigned(n) < buff.size()) {
            break;
        }
        /* Else try again with more space. */
        if (n > -1) { /* glibc 2.1 */ /* precisely what is needed */
            buff.resize(n+1, '\0');
        }
        else { /* glibc 2.0 */ /* twice the old size */
            buff.resize(buff.size()*2, '\0');
        }
    }
    std::string msg = CreateMessage(exp, file, line, func, &buff[0]);
    if (die) {
        puts(msg.c_str());
        abort();
    } else {
        throw AssertException(msg);
    }
}


