/* See LICENSE file for copyright and license details. */
#include "../slstatus.h"
#include "../util.h"

#include <err.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

const char *
datetime(const char *fmt)
{
	time_t t;

	t = time(NULL);
	if (strftime(buf, sizeof(buf), fmt, localtime(&t)) == 0 && strlen(fmt) > 0) {
		warnx("strftime: Result string exceeds buffer size");
		return NULL;
	}

	return buf;
}
