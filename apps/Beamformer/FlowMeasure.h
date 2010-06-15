
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

    double AverageRate() { return rate_sum/rates.size(); }
    double SmallestRate() { return smallest; }
    double LargestRate() { return largest; }
private:
    double last_time;
    std::deque<double> rates;
    double largest;
    double smallest;
    double rate_sum;
};
