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

#include "Time.h"

const long MICROPERSEC = 1000000;

// Add to 'compound' times.
void CompoundAdd(const unsigned long rhsec, const long rhusec,
		const unsigned long lhsec, const long lhusec,
		unsigned long &ressec, long &resusec) {
	ressec = rhsec + lhsec;
	resusec = rhusec + lhusec;
	// Loop will only happen once
	while (resusec >= MICROPERSEC) {
		ressec += 1;
		resusec -= MICROPERSEC;
	}
}

// Difference of the 'compound' time
void CompoundDiff(const unsigned long rhsec, const long rhusec,
		const unsigned long lhsec, const long lhusec,
		unsigned long &ressec, long &resusec) {
	if (rhsec > lhsec || (rhsec == lhsec && rhusec > lhusec)) {
		ressec = rhsec - lhsec;
		resusec = rhusec - lhusec;
	} else {
		ressec = lhsec - rhsec;
		resusec = lhusec - rhusec;
	}
	while (resusec < 0) {
		ressec -= 1;
		resusec += MICROPERSEC;
	}
}

const Time Time::Add(const TimeInterval& tmv) const {
	unsigned long sec;
	long usec;
	CompoundAdd(seconds, microseconds,
			tmv.Seconds(), tmv.Microseconds(),
			sec, usec);
	return Time(sec, usec);
}

const Time Time::Diff(const TimeInterval& tmv) const {
	unsigned long sec;
	long usec;
	CompoundDiff(seconds, microseconds,
			tmv.Seconds(), tmv.Microseconds(),
			sec, usec);
	return Time(sec, usec);
}

const TimeInterval Time::Diff(const Time& other) const {
	unsigned long sec;
	long usec;
	CompoundDiff(seconds, microseconds,
			other.Seconds(), other.Microseconds(),
			sec, usec);
	return TimeInterval(sec, usec);
}

TimeInterval::TimeInterval(const Time& first, const Time& second) {
	CompoundDiff(first.Seconds(), first.Microseconds(),
			second.Seconds(), second.Microseconds(),
			seconds, microseconds);
}

const TimeInterval TimeInterval::Diff(const TimeInterval& other) const {
	unsigned long sec;
	long usec;
	TimeInterval inter;
	CompoundDiff(seconds, microseconds,
			other.Seconds(), other.Microseconds(),
			sec, usec);
	return TimeInterval(sec, usec);
}

const TimeInterval TimeInterval::Add(const TimeInterval& other) const {
	unsigned long sec;
	long usec;
	TimeInterval inter;
	CompoundAdd(seconds, microseconds,
			other.Seconds(), other.Microseconds(),
			sec, usec);
	return TimeInterval(sec, usec);
}

const Time operator+(const TimeInterval& first, const Time& second) {
	return second.Add(first);
}

