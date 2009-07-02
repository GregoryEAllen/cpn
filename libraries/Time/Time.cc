/** \file
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

