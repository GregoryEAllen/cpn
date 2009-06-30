

#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdio>

void ReportClicks(void);
void ReportTimeOfDay(void);

int main(int argc, char **argv) {
	printf("Clicks per second %lu\n", sysconf(_SC_CLK_TCK));
	ReportTimeOfDay();
	ReportClicks();
	// sleep(1);
	ReportTimeOfDay();
	ReportClicks();
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

