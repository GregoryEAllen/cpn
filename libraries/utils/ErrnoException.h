
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

