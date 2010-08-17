
#include "Exception.h"
#include "StackTrace.h"

Exception::Exception(int ignore) throw()
    : stacktrace(GetStack(ignore))
{
}

Exception::~Exception() throw() {}

