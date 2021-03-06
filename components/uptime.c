/* See LICENSE file for copyright and license details. */
#include "../util.h"

#include <err.h>
#include <stdio.h>

#if defined(CLOCK_BOOTTIME)
	#define UPTIME_FLAG CLOCK_BOOTTIME
#elif defined(CLOCK_UPTIME)
	#define UPTIME_FLAG CLOCK_UPTIME
#else
	#define UPTIME_FLAG CLOCK_MONOTONIC
#endif

const char *
uptime(void)
{
	uintmax_t h, m;
	struct timespec uptime;

	if (clock_gettime(UPTIME_FLAG, &uptime) < 0) {
		warn("clock_gettime %d", UPTIME_FLAG);
		return NULL;
	}

	h = uptime.tv_sec / 3600;
	m = uptime.tv_sec % 3600 / 60;

	return bprintf("%juh %jum", h, m);
}
