/* See LICENSE file for copyright and license details. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "../util.h"

#include <math.h>
#include <time.h>

const char *
clockdiff(void)
{
	double old_time;
	static double new_time;
	struct timespec ts;

	old_time = new_time;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
		warn("clock_gettime:");
		return NULL;
	}

	new_time = timespec_to_double(&ts);

	if (old_time == 0) {
		return NULL;
	}

	return bprintf("%.6f", new_time - old_time);
}

const char *
clocktime(void)
{
	struct timespec ts;

	if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
		warn("clock_gettime:");
		return NULL;
	}

	return bprintf("%.6f", timespec_to_double(&ts));
}
