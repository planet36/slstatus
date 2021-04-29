/* See LICENSE file for copyright and license details. */
#include "../util.h"

#include <err.h>
#include <stdio.h>

const char *
datetime(const char *fmt)
{
	time_t t;

	t = time(NULL);
	if (!strftime(buf, sizeof(buf), fmt, localtime(&t))) {
		warnx("strftime: Result string exceeds buffer size");
		return NULL;
	}

	return buf;
}
