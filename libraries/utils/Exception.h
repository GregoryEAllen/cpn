
#ifndef EXCEPTION_H
#define EXCEPTION_H
#pragma once
#include <string>
#include <exception>

class Exception : public std::exception {
public:
    Exception(int ignore = 2) throw();
    virtual ~Exception() throw();
    std::string GetStackTrace() const { return stacktrace; }
protected:
    std::string stacktrace;
};

#endif
