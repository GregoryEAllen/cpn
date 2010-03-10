/** \file
 */

#ifndef THRESHOLDSIEVEOPTIOINS_H
#define THRESHOLDSIEVEOPTIOINS_H
#pragma once
#include "CPNCommon.h"
#include <vector>

struct ThresholdSieveOptions {
    typedef unsigned long long NumberT;
    NumberT maxprime;
    NumberT filtercount;
    unsigned long queuesize;
    unsigned long threshold;
    std::vector<double> primesPerFilter;
    unsigned long numPrimesSource;
    CPN::QueueHint_t queuehint;
    std::vector<NumberT> *results;
    CPN::Key_t consumerkey;
    bool report;
    bool zerocopy;
};

const char* const PRODUCER_NAME = "Producer";
const char* const FILTER_FORMAT = "Filter: %lu";
const char* const QUEUE_FORMAT = "Queue: %lu";
const char* const CONTROL_PORT = "Control Port";
const char* const IN_PORT = "x";
const char* const OUT_PORT = "y";

void CreateNewFilter(CPN::Kernel &kernel, ThresholdSieveOptions &opts, CPN::Key_t ourkey);

#endif

