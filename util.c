/* See LICENSE file for copyright and license details. */
#include "util.h"

#include <err.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *prefix_1000[] = { "", "k", "M", "G", "T", "P", "E", "Z",
                                     "Y" };
static const char *prefix_1024[] = { "", "Ki", "Mi", "Gi", "Ti", "Pi", "Ei",
                                     "Zi", "Yi" };

void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		(void)fputc(' ', stderr);
		perror(NULL);
	} else {
		(void)fputc('\n', stderr);
	}

	exit(1);
}

static int
evsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
	int ret;

	ret = vsnprintf(str, size, fmt, ap);

	if (ret < 0) {
		warn("vsnprintf");
		return -1;
	} else if ((size_t)ret >= size) {
		warnx("vsnprintf: Output truncated");
		return -1;
	}

	return ret;
}

int
esnprintf(char *str, size_t size, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = evsnprintf(str, size, fmt, ap);
	va_end(ap);

	return ret;
}

const char *
bprintf(const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = evsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	return (ret < 0) ? NULL : buf;
}

const char *
fmt_human(uintmax_t num, int base)
{
	long double scaled;
	size_t i, prefixlen;
	const char **prefix;

	switch (base) {
	case 1000:
		prefix = prefix_1000;
		prefixlen = LEN(prefix_1000);
		break;
	case 1024:
		prefix = prefix_1024;
		prefixlen = LEN(prefix_1024);
		break;
	default:
		warnx("fmt_human: Invalid base");
		return NULL;
	}

	scaled = num;
	for (i = 0; i < prefixlen && scaled >= base; i++) {
		scaled /= base;
	}

	return bprintf("%.1Lf %s", scaled, prefix[i]);
}

const char *
fmt_human_3(uintmax_t num, int base)
{
	long double scaled;
	size_t i, prefixlen;
	const char **prefix;
	int precision = 3;

	switch (base) {
	case 1000:
		prefix = prefix_1000;
		prefixlen = LEN(prefix_1000);
		break;
	case 1024:
		prefix = prefix_1024;
		prefixlen = LEN(prefix_1024);
		break;
	default:
		warnx("fmt_human: Invalid base");
		return NULL;
	}

	scaled = num;
	for (i = 0; i < prefixlen && scaled >= 999.5L; i++) {
		scaled /= base;
	}

	if (scaled < 1) {
		--precision;
	}

	return bprintf("%.*Lg %s", precision, scaled, prefix[i]);
}

int
pscanf(const char* path, const char* fmt, ...)
{
	FILE* fp;
	va_list ap;
	int n;

	fp = fopen(path, "r");
	if (fp == NULL)
	{
		warn("fopen '%s'", path);
		return -1;
	}
	va_start(ap, fmt);
	n = vfscanf(fp, fmt, ap);
	va_end(ap);
	if (fclose(fp) < 0)
		die("fclose:");

	return (n == EOF) ? -1 : n;
}

double
timespec_to_sec(const struct timespec* ts)
{
	return ts->tv_sec + copysign(ts->tv_nsec, ts->tv_sec) / 1E9;
}

struct timeval
msec_to_timeval(unsigned int msec)
{
	return (struct timeval){
	    .tv_sec = msec / 1000U,
	    .tv_usec = (msec % 1000U) * 1000UL,
	};
}
