
#include "FlowMeasure.h"
#include "ErrnoException.h"
#include <sys/time.h>
#include <limits>
#include <stdio.h>

static double getTime() {
    timeval tv;
    if (gettimeofday(&tv, 0) != 0) {
        throw ErrnoException();
    }
    return static_cast<double>(tv.tv_sec) + 1e-6 * static_cast<double>(tv.tv_usec);
}


FlowMeasure::FlowMeasure()
    : count_total(0), start(0), last_time(0),
    largest(0),
    smallest(std::numeric_limits<double>::infinity())
{
}

void FlowMeasure::Start() {
    start = last_time = getTime();
}

void FlowMeasure::Start(double time) {
    start = last_time = time;
}

void FlowMeasure::Tick(unsigned count) {
    Tick(count, getTime());
}

void FlowMeasure::Tick(unsigned count, double time) {
    count_total += count;
    double rate = count/(time - last_time);
    rates.push_back(rate);
    if (rate > largest) largest = rate;
    if (rate < smallest) smallest = rate;
    fprintf(stderr, "Tick(%p): r: %f, c: %u, t: %f\n", this, rate, count, time - last_time);
    last_time = time;
}

void FlowMeasure::Tick(TickInfo tick) {
    Tick(tick.count, tick.time);
}

FlowMeasure::TickInfo::TickInfo(unsigned c)
    : time(getTime()), count(c)
{}
