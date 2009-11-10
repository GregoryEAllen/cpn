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
 * \brief A very simple logging interface.
 */

#ifndef LOGGER_H
#define LOGGER_H
#pragma once
#include "ReentrantLock.h"
#include <string>
class LoggerOutput {
public:
    virtual ~LoggerOutput();
    virtual int LogLevel() const = 0;
    virtual int LogLevel(int level) = 0;
    virtual void Log(int level, const std::string &msg) const = 0;
};

class Logger : public LoggerOutput {
public:
    enum {
        ERROR = 100,
        WARNING = 75,
        INFO = 50,
        DEBUG = 25,
        TRACE = 0
    };
    Logger();
    Logger(int dfltlvl);
    Logger(const LoggerOutput *lo, int dfltlvl);
    Logger(const LoggerOutput *lo, int dfltlevel, const std::string &nm);

    int LogLevel() const;
    int LogLevel(int level);

    int DefaultLevel() const;
    int DefaultLevel(int level);

    int Adjust() const;
    int Adjust(int a);

    const std::string &Name() const;
    const std::string &Name(const std::string &nm);

    const LoggerOutput *Output() const;
    const LoggerOutput *Output(const LoggerOutput *output);

    virtual void Log(int level, const std::string &msg) const;

    void Log(const std::string &msg) const { Log(defaultlevel, msg); }

    void Logf(int level, const char *fmt, ...) const;

    void vLogf(int level, const char *fmt, va_list ap) const;

    void Logf(const char *fmt, ...) const;

    void Error(const char *fmt, ...) const;
    void Warn(const char *fmt, ...) const;
    void Info(const char *fmt, ...) const;
    void Debug(const char *fmt, ...) const;
    void Trace(const char *fmt, ...) const;
protected:
private:
    Logger(const Logger&);
    Logger &operator=(const Logger&);

    Sync::ReentrantLock lock;
    const LoggerOutput *logout;
    int loglevel;
    int defaultlevel;
    int adjust;
    std::string name;
};

class LoggerStdOutput : public LoggerOutput {
public:
    LoggerStdOutput(int level) : loglevel(level) {}
    int LogLevel(int level);
    int LogLevel() const;
    void Log(int level, const std::string &msg) const;
private:
    Sync::ReentrantLock lock;
    int loglevel;
};

struct ScopeTrace {
    ScopeTrace(const Logger &l, const char *fn, unsigned ln)
        : logger(l), fname(fn), line(ln)
    {
        logger.Trace("Enter %s:%u", fname, line);
    }
    ~ScopeTrace() {
        logger.Trace("Exit %s:%u", fname, line);
    }
    const Logger &logger;
    const char *fname;
    unsigned line;
};
// Will create a tracer on the stack which outputs a trace message of
// Enterying function and exiting function and it will use the symbol
// scoperace[linenumber]
#define SCOPE_TRACE(logger) ScopeTrace scopetracer__LINE__ (logger, __PRETTY_FUNCTION__, __LINE__)

#endif
