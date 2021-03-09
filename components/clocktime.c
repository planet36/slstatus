/* See LICENSE file for copyright and license details. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "../util.h"

#include <math.h>
#include <time.h>

static double
timespec_to_double(const struct timespec* ts)
{
	return ts->tv_sec + copysign(ts->tv_nsec, ts->tv_sec) / 1E9;
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
