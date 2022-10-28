// SPDX-FileCopyrightText: Steven Ward
// SPDX-License-Identifier: OSL-3.0

#include "../slstatus.h"
#include "../util.h"

#include <err.h>
#include <math.h>
#include <time.h>

const char *
clockdiff([[maybe_unused]] const char *unused)
{
	double prev_time;
	static double now_time;
	struct timespec ts;

	prev_time = now_time;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
		warn("clock_gettime");
		return NULL;
	}

	now_time = timespec_to_sec(&ts);

	if (prev_time == 0) {
		return NULL;
	}

	return bprintf("%.6f", now_time - prev_time);
}

const char *
clocktime([[maybe_unused]] const char *unused)
{
	struct timespec ts;

	if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
		warn("clock_gettime");
		return NULL;
	}

	return bprintf("%.6f", timespec_to_sec(&ts));
}
