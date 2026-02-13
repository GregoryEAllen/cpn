
#pragma once

#include <deque>

class FlowMeasure {
public:
    struct TickInfo {
        TickInfo(double t, unsigned c) : time(t), count(c) {}
        TickInfo(unsigned c);
        double time;
        unsigned count;
    };

    FlowMeasure();

    void Start();
    void Start(double time);
    void Tick(unsigned count);
    void Tick(unsigned count, double time);
    void Tick(TickInfo tick);

    double AverageRate() { return count_total/(last_time - start); }
    double SmallestRate() { return smallest; }
    double LargestRate() { return largest; }
private:
    unsigned count_total;
    double start;
    double last_time;
    std::deque<double> rates;
    double largest;
    double smallest;
};
