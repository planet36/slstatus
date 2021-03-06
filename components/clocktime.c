// SPDX-FileCopyrightText: Steven Ward
// SPDX-License-Identifier: OSL-3.0

#include "../util.h"

#include <err.h>
#include <math.h>

const char *
clockdiff(void)
{
	double prev_time;
	static double now_time;
	struct timespec ts;

	prev_time = now_time;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
		warn("clock_gettime");
		return NULL;
	}

	now_time = timespec_to_double(&ts);

	if (prev_time == 0) {
		return NULL;
	}

	return bprintf("%.6f", now_time - prev_time);
}

const char *
clocktime(void)
{
	struct timespec ts;

	if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
		warn("clock_gettime");
		return NULL;
	}

	return bprintf("%.6f", timespec_to_double(&ts));
}
