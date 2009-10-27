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
#ifndef ERRNOEXCEPTION_H
#define ERRNOEXCEPTION_H
#pragma once

#include <exception>
#include <vector>
#include <string>

/**
 * A generic exception that encapsulates
 * the errno value.
 *
 * Note that this dynamically allocates memory
 * so is unsutable for out of memory errors.
 */
class ErrnoException : public std::exception {
public:
    ErrnoException() throw();
    ErrnoException(int err) throw();
    ErrnoException(const char *msg, int err) throw();
    virtual ~ErrnoException() throw();
    virtual const char* what() const throw();
    int Error() const { return error; }
private:
    void Fill() throw();
    int error;
    std::string errorstring;
};
#endif
