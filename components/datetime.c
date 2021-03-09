/* See LICENSE file for copyright and license details. */
#include "../util.h"

#include <stdio.h>
#include <time.h>

const char *
datetime(const char *fmt)
{
	time_t t;

	t = time(NULL);
	if (!strftime(buf, sizeof(buf), fmt, localtime(&t))) {
		warn("strftime: Result string exceeds buffer size");
		return NULL;
	}

	return buf;
}
