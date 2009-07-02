

#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdio>
#include "Time.h"

void ReportClicks(void);
void ReportTimeOfDay(void);

int main(int argc, char **argv) {
	//printf("Clicks per second %lu\n", sysconf(_SC_CLK_TCK));
	Time start;
	printf("Time: %s\n", Time().ToString().c_str());
	//ReportTimeOfDay();
	//ReportClicks();
	sleep(1);
	//ReportTimeOfDay();
	//ReportClicks();
	TimeInterval timeinterval = start.Elapsed();
	printf("Time class: %s\n", timeinterval.ToString().c_str());
	Time test;
	TimeInterval testInterval = test.Elapsed();
	printf("Time class noops test: %s\n", testInterval.ToString().c_str());
	printf("Time: %s\n", Time().ToString().c_str());
	return 0;
}


void ReportClicks(void) {
	tms time;
	clock_t t = times(&time);
	printf("Clicks: %lu\n", t);
	printf("User clicks: %lu\n", time.tms_utime);
	printf("Sys clicks: %lu\n", time.tms_stime);
}

void ReportTimeOfDay(void) {
	timeval t;
	gettimeofday(&t, 0);
	printf("Secs: %lu\n", t.tv_sec);
	printf("uSec: %lu\n", t.tv_usec);
}

