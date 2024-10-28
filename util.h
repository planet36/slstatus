/* See LICENSE file for copyright and license details. */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <time.h>

extern char buf[1024];

#define LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

int esnprintf(char *str, size_t size, const char *fmt, ...);
const char *bprintf(const char *fmt, ...);
const char *fmt_human(uintmax_t num, int base);
const char *fmt_human_3(uintmax_t num, int base);
int pscanf(const char *path, const char *fmt, ...);

double timespec_to_sec(const struct timespec *ts);
struct timeval msec_to_timeval(unsigned int msec);
