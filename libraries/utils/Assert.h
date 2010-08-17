//=============================================================================
//Computational Process Networks class library Copyright (C) 1997-2006  Gregory
//E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or (at your
//	option) any later version.
//
//	This library is distributed in the hope that it will be useful, but WITHOUT
//	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//	FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//	License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you can write
//	to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
//	MA 02111-1307, USA, or you can find it on the World Wide Web at
//	http://www.fsf.org.
//=============================================================================
/** \file
 *
 * An implementation of an assert like micro that throws an exception.  This is
 * usefull when using a unit test as the test will then fail and the
 * application will continue.
 *
 * Note that this file uses some GCC only features like exp ?: see
 * http://gcc.gnu.org/onlinedocs/gcc-4.4.1/gcc/Conditionals.html
 *
 * Use ASSERT when you want the test to be compiled out when NDEBUG is defined
 * and use ENSURE when you want the expression to remain when NDEBUG is
 * defined.
 *
 * Also these macros evaluate there arguments only ONCE.
 *
 * \author John Bridgman
 */
#ifndef ASSERT_H
#define ASSERT_H
#pragma once

#include "Exception.h"

/**
 * \brief The exception thrown by the ASSERT macro.
 */
class AssertException : public Exception {
public:
    AssertException(const std::string &msg) throw();
    virtual ~AssertException() throw();
    virtual const char *what() const throw();
private:
    std::string message;
};

bool __ASSERT(const char *exp, const char *file, int line, const char *func, bool die) __attribute__((noreturn));

bool __ASSERT(const char *exp, const char *file, int line, const char *func, bool die, const std::string &msg) __attribute__((noreturn));

bool __ASSERT(const char *exp, const char *file, int line, const char *func, bool die, const char *fmt, ...) __attribute__((noreturn));

#ifndef NDEBUG
#define ASSERT(exp, ...) (exp ? true : __ASSERT(#exp, __FILE__, __LINE__, __PRETTY_FUNCTION__, false, ## __VA_ARGS__))
#define ASSERT_ABORT(exp, ...) (exp ? true : __ASSERT(#exp, __FILE__, __LINE__, __PRETTY_FUNCTION__, true, ## __VA_ARGS__))
#define ENSURE(exp, ...) (exp ? true : __ASSERT(#exp, __FILE__, __LINE__, __PRETTY_FUNCTION__, false, ## __VA_ARGS__))
#define ENSURE_ABORT(exp, ...) (exp ? true : __ASSERT(#exp, __FILE__, __LINE__, __PRETTY_FUNCTION__, true, ## __VA_ARGS__))
#else
#define ASSERT(exp, ...)
#define ASSERT_ABORT(exp, ...)
#define ENSURE(exp, ...) exp
#define ENSURE_ABORT(exp, ...) exp
#endif

#endif

