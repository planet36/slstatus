/* See LICENSE file for copyright and license details. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "../util.h"

#include <stdio.h>
#include <stdlib.h>

const char *
load_avg(void)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		warn("getloadavg: Failed to obtain load average");
		return NULL;
	}

	return bprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}
