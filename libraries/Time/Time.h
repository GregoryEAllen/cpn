/** \file
 * A class and set of utility functions to deal
 * with high precision time(us). This is meant for
 * profiling purposes.
 *
 * Here we assume time_t and susecond_t are
 * unsigned long and long
 */

#ifndef TIME_H
#define TIME_H

#include <sys/time.h>

class TimeInterval;

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
	const Time &operator=(const Time& other) const { return Time(other); }
	const Time &Add(const TimeInterval& tmv) const;
	const Time &Diff(const TimeInterval& tmv) const;
	const TimeInterval &Diff(const Time& other) const;
	const TimeInterval &Elapsed(void) const { return Diff(Time()); }
	unsigned long Seconds(void) const { return seconds; }
	long Microseconds(void) const { return microseconds; }
	bool operator<(const Time& other) const { return (seconds < other.seconds)
	       	|| (seconds == other.seconds && microseconds < other.microseconds); } 
	bool operator==(const Time& other) const { return (seconds == other.seconds)
		&& (microseconds == other.microseconds); }
private:
	unsigned long seconds;
	long microseconds;
};

class TimeInterval {
public:
	TimeInterval() : seconds(0), microseconds(0) {}
	TimeInterval(unsigned long seconds_, long microseconds_)
		: seconds(seconds_), microseconds(microseconds_) {}
	TimeInterval(const Time& first, const Time& second);
	TimeInterval(const TimeInterval &other)
		: seconds(other.seconds), microseconds(other.microseconds) {}
	const TimeInterval &operator=(const TimeInterval& other) { return TimeInterval(other); }
	const TimeInterval &Diff(const TimeInterval& other) const;
	const TimeInterval &Add(const TimeInterval& other) const;
	unsigned long Seconds(void) const { return seconds; }
	long Microseconds(void) const { return microseconds; } 
	bool operator<(const TimeInterval& other) const { return (seconds < other.seconds)
	       	|| (seconds == other.seconds && microseconds < other.microseconds); } 
	bool operator==(const TimeInterval& other) const { return (seconds == other.seconds)
		&& (microseconds == other.microseconds); } 
private:
	unsigned long seconds;
	long microseconds;
};

const TimeInterval &operator-(const Time& first, const Time& second) {
	return first.Diff(second);
}
const Time &operator-(const Time& first, const TimeInterval& second) {
	return first.Diff(second);
}
const TimeInterval &operator-(const TimeInterval& first, const TimeInterval& second) {
	return first.Diff(second);
}
const Time &operator+(const Time& first, const TimeInterval& second) {
	return first.Add(second);
}
const Time &operator+(const TimeInterval& first, const Time& second) {
	return second.Add(first);
}
const TimeInterval &operator+(const TimeInterval& first, const TimeInterval& second) {
	return first.Add(second);
}

bool operator!= (const Time& x, const Time& y) { return !(x==y); }
bool operator>  (const Time& x, const Time& y) { return y<x; }
bool operator<= (const Time& x, const Time& y) { return !(y<x); }
bool operator>= (const Time& x, const Time& y) { return !(x<y); }
bool operator!= (const TimeInterval& x, const TimeInterval& y) { return !(x==y); }
bool operator>  (const TimeInterval& x, const TimeInterval& y) { return y<x; }
bool operator<= (const TimeInterval& x, const TimeInterval& y) { return !(y<x); }
bool operator>= (const TimeInterval& x, const TimeInterval& y) { return !(x<y); }


#endif

