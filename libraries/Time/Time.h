//=============================================================================
//  Computational Process Networks class library
//  Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//  This library is free software; you can redistribute it and/or modify it
//  under the terms of the GNU Library General Public License as published
//  by the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Library General Public License for more details.
//
//  The GNU Public License is available in the file LICENSE, or you
//  can write to the Free Software Foundation, Inc., 59 Temple Place -
//  Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//  World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * A class and set of utility functions to deal
 * with high precision time(us). This is meant for
 * profiling purposes.
 *
 * Here we assume time_t and susecond_t are
 * unsigned long and long
 *
 * \author John Bridgman
 */

#ifndef TIME_H
#define TIME_H

#include <sys/time.h>
#include "ToString.h"

class Time;

/**
 * Represents an interval of time. Note that a time interval
 * is alway positive. That is to say
 * <code>
 * Time tm1, tm2;
 * assert(tm1 - tm2 == tm2 - tm1);
 * </code>
 */
class TimeInterval {
public:
    TimeInterval() : seconds(0), microseconds(0) {}
    TimeInterval(unsigned long seconds_, long microseconds_)
        : seconds(seconds_), microseconds(microseconds_) { }
    TimeInterval(const Time& first, const Time& second);
    TimeInterval(const TimeInterval &other)
        : seconds(other.seconds), microseconds(other.microseconds) {}
    const TimeInterval operator=(const TimeInterval& other) { return TimeInterval(other); }
    const TimeInterval Diff(const TimeInterval& other) const;
    const TimeInterval Add(const TimeInterval& other) const;
    unsigned long Seconds(void) const { return seconds; }
    long Microseconds(void) const { return microseconds; } 
    bool operator<(const TimeInterval& other) const { return (seconds < other.seconds)
            || (seconds == other.seconds && microseconds < other.microseconds); } 
    bool operator==(const TimeInterval& other) const { return (seconds == other.seconds)
        && (microseconds == other.microseconds); } 
    bool operator!= (const TimeInterval& y) { return !(*this==y); }
    bool operator>  (const TimeInterval& y) { return y<*this; }
    bool operator<= (const TimeInterval& y) { return !(y<*this); }
    bool operator>= (const TimeInterval& y) { return !(*this<y); }
    const TimeInterval operator-(const TimeInterval& second) { return Diff(second); }
    const TimeInterval operator+(const TimeInterval& second) { return Add(second); }
    std::string ToString() const { 
        return ::ToString("%lu.%06ld", seconds, microseconds); }
private:
    unsigned long seconds;
    long microseconds;
};

/**
 * Convenience class for working with high precision time in
 * profiles.
 */
class Time {
public:

    Time() {
        timeval theTime;
        gettimeofday(&theTime, 0);
        seconds = theTime.tv_sec;
        microseconds = theTime.tv_usec;
    }
    Time(unsigned long seconds_, long microseconds_)
        : seconds(seconds_), microseconds(microseconds_) {}
    Time(const Time& other)
        : seconds(other.seconds), microseconds(other.microseconds) {}
    const Time operator=(const Time& other) const { return Time(other); }
    const Time Add(const TimeInterval& tmv) const;
    const Time Diff(const TimeInterval& tmv) const;
    const TimeInterval Diff(const Time& other) const;
    const TimeInterval Elapsed(void) const { return Diff(Time()); }
    unsigned long Seconds(void) const { return seconds; }
    long Microseconds(void) const { return microseconds; }
    bool operator<(const Time& other) const { return (seconds < other.seconds)
            || (seconds == other.seconds && microseconds < other.microseconds); } 
    bool operator==(const Time& other) const { return (seconds == other.seconds)
        && (microseconds == other.microseconds); }
    bool operator!= (const Time& y) { return !(*this==y); }
    bool operator>  (const Time& y) { return y<*this; }
    bool operator<= (const Time& y) { return !(y<*this); }
    bool operator>= (const Time& y) { return !(*this<y); }
    const TimeInterval operator-(const Time& second) { return Diff(second); }
    const Time operator-(const TimeInterval& second) { return Diff(second); }
    const Time operator+(const TimeInterval& second) { return Add(second); }
    std::string ToString() const { 
        return ::ToString("%lu.%06ld", seconds, microseconds); }
private:
    unsigned long seconds;
    long microseconds;
};


const Time operator+(const TimeInterval& first, const Time& second);


#endif

