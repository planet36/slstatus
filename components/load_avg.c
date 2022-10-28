/* See LICENSE file for copyright and license details. */
#include "../slstatus.h"
#include "../util.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

const char *
load_avg([[maybe_unused]] const char *unused)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		warnx("getloadavg: Failed to obtain load average");
		return NULL;
	}

	return bprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}
