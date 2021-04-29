/* See LICENSE file for copyright and license details. */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <time.h>

extern char buf[1024];

#define LEN(x) (sizeof(x) / sizeof((x)[0]))

void die(const char *, ...) __attribute__ ((noreturn));

int esnprintf(char *str, size_t size, const char *fmt, ...);
const char *bprintf(const char *fmt, ...);
const char *fmt_human(uintmax_t num, int base);
const char *fmt_human_3(uintmax_t num, int base);
int pscanf(const char *path, const char *fmt, ...);

double timespec_to_double(const struct timespec *ts);
struct timeval milliseconds_to_timeval(unsigned int milliseconds);
