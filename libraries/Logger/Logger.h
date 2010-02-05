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
#include <cstdarg>
/**
 * \brief Abstract base class for logger outputers.
 * Any object who wishes to be a place for logging messages to go
 * inherits from this class.
 */
class LoggerOutput {
public:
    virtual ~LoggerOutput();
    /** \return the current log level
     */
    virtual int LogLevel() const = 0;
    /** \param level the new log level
     * \return the new log level
     */
    virtual int LogLevel(int level) = 0;
    /** \brief Log a message to this outputer
     * \param level the level of this message
     * \param msg the message
     */
    virtual void Log(int level, const std::string &msg) = 0;
};

/**
 * \brief Logger object that is used for forwarding log messages.
 */
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
    Logger(LoggerOutput *lo, int dfltlvl);
    Logger(LoggerOutput *lo, int dfltlevel, const std::string &nm);
    virtual ~Logger();

    int LogLevel() const;
    int LogLevel(int level);

    int DefaultLevel() const;
    int DefaultLevel(int level);

    int Adjust() const;
    int Adjust(int a);

    const std::string &Name() const;
    const std::string &Name(const std::string &nm);

    LoggerOutput *Output();
    LoggerOutput *Output(LoggerOutput *output);

    virtual void Log(int level, const std::string &msg);

    void Log(const std::string &msg) { Log(defaultlevel, msg); }

    void Logf(int level, const char *fmt, ...);

    void vLogf(int level, const char *fmt, va_list ap);

    void Logf(const char *fmt, ...);

    void Error(const char *fmt, ...);
    void Warn(const char *fmt, ...);
    void Info(const char *fmt, ...);
    void Debug(const char *fmt, ...);
    void Trace(const char *fmt, ...);
protected:
    Logger(const Logger&);
    Logger &operator=(const Logger&);
private:

    Sync::ReentrantLock lock;
    LoggerOutput *logout;
    int loglevel;
    int defaultlevel;
    int adjust;
    std::string name;
};

/**
 * \brief A LoggerOutput implementation that prints to stdout
 */
class LoggerStdOutput : public LoggerOutput {
public:
    LoggerStdOutput(int level) : loglevel(level) {}
    int LogLevel(int level);
    int LogLevel() const;
    void Log(int level, const std::string &msg);
private:
    Sync::ReentrantLock lock;
    int loglevel;
};

struct ScopeTrace {
    ScopeTrace(Logger &l, const char *fn, unsigned ln)
        : logger(l), fname(fn), line(ln)
    {
        logger.Trace("Enter %s:%u", fname, line);
    }
    ~ScopeTrace() {
        logger.Trace("Exit %s:%u", fname, line);
    }
    Logger &logger;
    const char *fname;
    unsigned line;
};
// Will create a tracer on the stack which outputs a trace message of
// Enterying function and exiting function and it will use the symbol
// scoperace[linenumber]
#define SCOPE_TRACE(logger) ScopeTrace scopetracer__LINE__ (logger, __PRETTY_FUNCTION__, __LINE__)

#endif
