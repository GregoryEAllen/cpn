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

