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

#include "ErrnoException.h"
#include <errno.h>
#include <string.h>

static const char UNKNOWN_ERROR[] = "Unknown error";

ErrnoException::ErrnoException() throw()
    : error(errno)
{
    Fill();
}

ErrnoException::ErrnoException(int err) throw()
    : error(err)
{
    Fill();
}

ErrnoException::ErrnoException(const char *msg, int err) throw()
    : error(err)
{
    errorstring = msg;
}

ErrnoException::~ErrnoException() throw() {
}

const char* ErrnoException::what() const throw() {
    return errorstring.c_str();
}

void ErrnoException::Fill() throw() {
    std::vector<char> errstr(256, '\0');
    do {
#if defined(OS_DARWIN)
        // if OS_DARWIN strerror_r is declared to return an int
        int err = strerror_r(error, &errstr[0], errstr.size());
        if (err == 0) {
            errorstring = &errstr[0];
            break;
        } else {
            if (errno == ERANGE) {
                errstr.resize(2*errstr.size(), '\0');
            } else {
                errorstring = UNKNOWN_ERROR;
                break;
            }
        }
#else
        char *str = strerror_r(error, &errstr[0], errstr.size());

        // Wierdness with different versions of strerror... From the man page:
        //
        //  The strerror() and strerror_r() functions return the appropriate
        //  error description string, or an "Unknown error nnn" message if the
        //  error number is unknown.
        //
        //  The XSI-compliant strerror_r() function returns 0 on success; on
        //  error, -1 is returned and errno is set to indicate the error.
        //
        // So str can be ether an error code OR a pointer to the error
        // string...  wierdness.

        if (str == (char*)-1) {
            if (errno == ERANGE) {
                errstr.resize(2*errstr.size(), '\0');
            } else {
                errorstring = UNKNOWN_ERROR;
                break;
            }
        } else if (str == 0) {
            errorstring = &errstr[0];
            break;
        } else {
            errorstring = str;
            break;
        }
#endif
    } while (true);
}

