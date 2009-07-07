/** \file
 */

#ifndef THRESHOLDSIEVEOPTIOINS_H
#define THRESHOLDSIEVEOPTIOINS_H
#include <vector>
struct ThresholdSieveOptions {
	unsigned long maxprime;
	unsigned long queuesize;
	unsigned long threshold;
	std::string queueTypeName;
	std::vector<unsigned long> *results;
};

const char* const PRODUCER_NAME = "Producer";
const char* const CONSUMERQ_NAME = "Consumer Queue";
const char* const FILTER_FORMAT = "Filter: %lu";
const char* const QUEUE_FORMAT = "Queue: %lu";
const char* const IN_PORT = "x";
const char* const OUT_PORT = "y";

#endif

