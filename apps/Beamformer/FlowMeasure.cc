
#include "FlowMeasure.h"
#include "ErrnoException.h"
#include <sys/time.h>
#include <limits>

static double getTime() {
    timeval tv;
    if (gettimeofday(&tv, 0) != 0) {
        throw ErrnoException();
    }
    return static_cast<double>(tv.tv_sec) + 1e-6 * static_cast<double>(tv.tv_usec);
}


FlowMeasure::FlowMeasure()
    : last_time(0),
    largest(0),
    smallest(std::numeric_limits<double>::infinity()),
    rate_sum(0)
{
}

void FlowMeasure::Start() {
    last_time = getTime();
}

void FlowMeasure::Start(double time) {
    last_time = time;
}

void FlowMeasure::Tick(unsigned count) {
    Tick(count, getTime());
}

void FlowMeasure::Tick(unsigned count, double time) {
    double rate = count/(time - last_time);
    rates.push_back(rate);
    rate_sum += rate;
    if (rate > largest) largest = rate;
    if (rate < smallest) smallest = rate;
    last_time = time;
}

void FlowMeasure::Tick(TickInfo tick) {
    Tick(tick.count, tick.time);
}

FlowMeasure::TickInfo::TickInfo(unsigned c)
    : time(getTime()), count(c)
{}
