

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
    } while (true);
}

