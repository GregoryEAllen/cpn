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

#include "Logger.h"
#include "AutoBuffer.h"
#include "Assert.h"
#include <iostream>
#include <stdarg.h>

LoggerOutput::~LoggerOutput() {
}

Logger::Logger()
    : logout(0), loglevel(WARNING), defaultlevel(INFO)
{}

Logger::Logger(int dfltlvl)
    : logout(0), loglevel(WARNING), defaultlevel(dfltlvl)
{}

Logger::Logger(const LoggerOutput *lo, int dfltlvl)
    : logout(lo), loglevel(WARNING), defaultlevel(dfltlvl)
{
    ASSERT(logout);
    loglevel = logout->LogLevel();
}

Logger::Logger(const LoggerOutput *lo, int dfltlevel, const std::string &nm)
    : logout(lo), loglevel(WARNING), defaultlevel(dfltlevel), name(nm)
{
    ASSERT(logout);
    loglevel = logout->LogLevel();
}

int Logger::LogLevel() const {
    Sync::AutoReentrantLock arl(lock);
    return loglevel;
}

int Logger::LogLevel(int level) {
    Sync::AutoReentrantLock arl(lock);
    return loglevel = level;
}

int Logger::DefaultLevel() const {
    Sync::AutoReentrantLock arl(lock);
    return defaultlevel;
}

int Logger::DefaultLevel(int level) {
    Sync::AutoReentrantLock arl(lock);
    return defaultlevel = level;
}

const std::string &Logger::Name() const {
    Sync::AutoReentrantLock arl(lock);
    return name;
}

const std::string &Logger::Name(const std::string &nm) {
    Sync::AutoReentrantLock arl(lock);
    return name = nm;
}

const LoggerOutput *Logger::Output() const {
    Sync::AutoReentrantLock arl(lock);
    return logout;
}

const LoggerOutput *Logger::Output(const LoggerOutput *output) {
    Sync::AutoReentrantLock arl(lock);
    return logout = output;
}

void Logger::Log(int level, const std::string &msg) const {
    Sync::AutoReentrantLock arl(lock);
    if (level < loglevel) { return; }
    if (logout) { logout->Log(level, name + ":" + msg); }
}

void Logger::Logf(int level, const char *fmt, ...) const {
    Sync::AutoReentrantLock arl(lock);
    if (level < loglevel) { return; }
    va_list ap;
    va_start(ap, fmt);
    vLogf(level, fmt, ap);
    va_end(ap);
}

void Logger::Logf(const char *fmt, ...) const {
    Sync::AutoReentrantLock arl(lock);
    if (defaultlevel < loglevel) { return; }
    va_list ap;
    va_start(ap, fmt);
    vLogf(defaultlevel, fmt, ap);
    va_end(ap);
}

void Logger::vLogf(int level, const char *fmt, va_list ap) const {
    Sync::AutoReentrantLock arl(lock);
    if (level < loglevel) { return; }
    // This code was based on an example of how
    // to use vsnprintf in the unix man pages.
    AutoBuffer buff(128);
    while (1) {
        /* Try to print in the allocated space. */
        va_list ap_copy;
        va_copy(ap_copy, ap);
        int n = vsnprintf((char*)buff.GetBuffer(), buff.GetSize(), fmt, ap_copy);
        /* If that worked, return the string. */
        if (n > -1 && unsigned(n) < buff.GetSize()) {
            std::string ret = (char*)buff.GetBuffer();
            Log(level, ret);
            return;
        }
        /* Else try again with more space. */
        if (n > -1) { /* glibc 2.1 */ /* precisely what is needed */
            buff.ChangeSize(n+1);
        }
        else { /* glibc 2.0 */ /* twice the old size */
            buff.ChangeSize(buff.GetSize()*2);
        }
    }
}

void Logger::Error(const char *fmt, ...) const {
    Sync::AutoReentrantLock arl(lock);
    if (ERROR < loglevel) { return; }
    va_list ap;
    va_start(ap, fmt);
    vLogf(ERROR, fmt, ap);
    va_end(ap);
}

void Logger::Warn(const char *fmt, ...) const {
    Sync::AutoReentrantLock arl(lock);
    if (WARNING < loglevel) { return; }
    va_list ap;
    va_start(ap, fmt);
    vLogf(WARNING, fmt, ap);
    va_end(ap);
}

void Logger::Info(const char *fmt, ...) const {
    Sync::AutoReentrantLock arl(lock);
    if (INFO < loglevel) { return; }
    va_list ap;
    va_start(ap, fmt);
    vLogf(INFO, fmt, ap);
    va_end(ap);
}

void Logger::Debug(const char *fmt, ...) const {
    Sync::AutoReentrantLock arl(lock);
    if (DEBUG < loglevel) { return; }
    va_list ap;
    va_start(ap, fmt);
    vLogf(DEBUG, fmt, ap);
    va_end(ap);
}

void Logger::Trace(const char *fmt, ...) const {
    Sync::AutoReentrantLock arl(lock);
    if (TRACE < loglevel) { return; }
    va_list ap;
    va_start(ap, fmt);
    vLogf(TRACE, fmt, ap);
    va_end(ap);
}

int LoggerStdOutput::LogLevel(int level) {
    Sync::AutoReentrantLock arl(lock);
    return loglevel = level;
}

int LoggerStdOutput::LogLevel() const {
    Sync::AutoReentrantLock arl(lock);
    return loglevel;
}

void LoggerStdOutput::Log(int level, const std::string &msg) const {
    Sync::AutoReentrantLock arl(lock);
    if (level >= loglevel) {
        std::cout << level << ":" << msg << std::endl;
    }
}

