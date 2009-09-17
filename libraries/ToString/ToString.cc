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
#include "ToString.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// This code was taken from an example of how
// to use vsnprintf in the unix man pages.
std::string ToString(const char *fmt, ...)
{   
	/* Guess we need no more than 100 bytes. */
	int n, size = 100;
	char *p, *np;
	va_list ap;
	std::string ret = "";
	p = (char*)malloc(size);
	if (p == NULL) {
		return ret;
	}

	while (1) {
		/* Try to print in the allocated space. */
		va_start(ap, fmt);
		n = vsnprintf(p, size, fmt, ap);
		va_end(ap);
		/* If that worked, return the string. */
		if (n > -1 && n < size) {
			ret = p;
			free(p);
			return ret;
		}
		/* Else try again with more space. */
		if (n > -1) {
		    	/* glibc 2.1 */
			size = n+1; /* precisely what is needed */
		}
		else {          /* glibc 2.0 */
			size *= 2;  /* twice the old size */
		}
		np = (char*)realloc (p, size);
		if (np == NULL) {
			free(p);
			return ret;
		} else {
			p = np;
		}
	}
}

