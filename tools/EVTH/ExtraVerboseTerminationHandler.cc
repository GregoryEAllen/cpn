#include "ExtraVerboseTerminationHandler.h"
#include <exception>
#include <cxxabi.h>
#include <execinfo.h>
#include <typeinfo>
#include <stdlib.h>
#include <stdio.h>

/** \file
 *
 * This termination handler is based on the termination handler from
 * the gnu libstdc++ library. The file I took most of this code from
 * is located in the libstc++-v3/libsupc++/vterminate.cc file.
 *
 * The main reason I want this is I want it to print out a backtrace.
 *
 * The following is the license from the source file:
 
// Copyright (C) 2001, 2002, 2004 Free Software Foundation
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

 *
 * \author John Bridgman
 */
void extra_verbose_termination_handler() {
    static bool terminating = false;
    void *stack[100];
    if (terminating) {
        fputs("terminate called recursively\n", stderr);
    } else {
        terminating = true;
        std::type_info *t = abi::__cxa_current_exception_type();
        if (t) {
            const char *name = t->name();
            int status = -1;
            char *dem = 0;
            dem = abi::__cxa_demangle(name, 0, 0, &status);
            fputs("terminate called after throwing an instance of '", stderr);
            if (status == 0) {
                fputs(dem, stderr);
                free(dem);
            } else {
                fputs(name, stderr);
            }
            fputs("'\n", stderr);
            try {
                throw;
            } catch (const std::exception &e) {
                fputs("    what(): ", stderr);
                fputs(e.what(), stderr);
                fputs("\n", stderr);
            } catch(...) {
            }
        } else {
            fputs("terminate called without an active exception\n", stderr);
        }
    }
    fputs("\nbacktrace:\n", stderr);
    size_t size = backtrace(stack, 100);
    backtrace_symbols_fd(stack, size, fileno(stderr));
    abort();
}

