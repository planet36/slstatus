/* See LICENSE file for copyright and license details. */
#include "../util.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
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

const char *
datetimetz(const char *fmt)
{
	size_t tz_len;
	const char *tz_old;
	const char *tz_new;
	const char *result;

	if (strncmp(fmt, "TZ=", strlen("TZ=")) != 0)
		return datetime(fmt);

	fmt += strlen("TZ=");
	tz_len = strcspn(fmt, " "); // count non-spaces
	tz_new = strndupa(fmt, tz_len);
	fmt += tz_len;
	fmt += strspn(fmt, " "); // count spaces

	tz_old = getenv("TZ");

	if (setenv("TZ", tz_new, 1) < 0)
		warn("setenv \"TZ\" \"%s\"", tz_new);

	tzset();

	result = datetime(fmt);

	if (tz_old != NULL) {
		if (setenv("TZ", tz_old, 1) < 0)
			warn("setenv \"TZ\" \"%s\"", tz_old);
	}
	else {
		if (unsetenv("TZ") < 0)
			warn("unsetenv \"TZ\"");
	}

	tzset();

	return result;
}
