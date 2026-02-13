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
 * \brief An exception thrown when a true deadlock is detected
 * \author John Bridgman
 */

#ifndef D4R_DEADLOCKEXCEPTION_H
#define D4R_DEADLOCKEXCEPTION_H
#pragma once

#include <string>
#include <exception>

namespace D4R {
    /**
     * \brief The exception thrown when true deadlock is detected.
     */
    class DeadlockException : public std::exception {
    public:
        DeadlockException(const std::string &msg) throw();
        virtual ~DeadlockException() throw();
        const char *what() const throw();
    private:
        const std::string message;
    };
}
#endif
