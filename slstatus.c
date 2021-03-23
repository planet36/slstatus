/* See LICENSE file for copyright and license details. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "slstatus.h"
#include "util.h"

#include <X11/Xlib.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

struct arg {
	const char *(*func)();
	const char *fmt;
	const char *args;
};

char buf[1024];
double delta_time = 0; // seconds
static volatile sig_atomic_t done;
static Display *dpy;

#include "config.h"

static void
terminate(const int signo)
{
	if (signo != SIGUSR1)
		done = 1;
}

static void
difftimespec(struct timespec *res, struct timespec *a, struct timespec *b)
{
	res->tv_sec = a->tv_sec - b->tv_sec - (a->tv_nsec < b->tv_nsec);
	res->tv_nsec = a->tv_nsec - b->tv_nsec +
	               (a->tv_nsec < b->tv_nsec) * 1E9;
}

static void
usage(const char* argv0)
{
	printf("usage: %s [-h] [-1] [-s] [-v]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int ch;
	const char *optstring = "+h1sv";
	struct sigaction act;
	struct timespec start, current, diff, intspec, wait;
	size_t i, len;
	int sflag, ret;
	char status[MAXLEN];
	const char *res;

	sflag = 0;
	while ((ch = getopt(argc, argv, optstring)) != -1) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
			break;
		case '1':
			done = 1;
			/* fallthrough */
		case 's':
			sflag = 1;
			break;
		case 'v':
			printf("%s " VERSION "\n", argv[0]);
			return 0;
			break;
		default:
			usage(argv[0]);
			return 1;
			break;
		}
	}

	argc -= optind;
	//argv += optind;

	if (argc) {
		usage(argv[0]);
		return 1;
	}

	intspec.tv_sec = interval / 1000;
	intspec.tv_nsec = (interval % 1000) * 1E6;

	memset(&act, 0, sizeof(act));
	act.sa_handler = terminate;
	sigaction(SIGINT,  &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	act.sa_flags |= SA_RESTART;
	sigaction(SIGUSR1, &act, NULL);

	if (!sflag && !(dpy = XOpenDisplay(NULL))) {
		die("XOpenDisplay: Failed to open display");
	}

	do {
		if (clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
			die("clock_gettime:");
		}

		status[0] = '\0';
		for (i = len = 0; i < LEN(args); i++) {
			if (!(res = args[i].func(args[i].args))) {
				res = unknown_str;
			}
			if ((ret = esnprintf(status + len, sizeof(status) - len,
			                    args[i].fmt, res)) < 0) {
				break;
			}
			len += ret;
		}

		if (sflag) {
			puts(status);
			fflush(stdout);
			if (ferror(stdout))
				die("puts:");
		} else {
			if (XStoreName(dpy, DefaultRootWindow(dpy), status)
                            < 0) {
				die("XStoreName: Allocation failed");
			}
			XFlush(dpy);
		}

		if (!done) {
			if (clock_gettime(CLOCK_MONOTONIC, &current) < 0) {
				die("clock_gettime:");
			}
			difftimespec(&diff, &current, &start);
			difftimespec(&wait, &intspec, &diff);
			delta_time = timespec_to_double(&intspec);

			if (wait.tv_sec >= 0) {
				if (nanosleep(&wait, NULL) < 0 &&
				    errno != EINTR) {
					die("nanosleep:");
				}
			}
		}
	} while (!done);

	if (!sflag) {
		XStoreName(dpy, DefaultRootWindow(dpy), NULL);
		if (XCloseDisplay(dpy) < 0) {
			die("XCloseDisplay: Failed to close display");
		}
	}

	return 0;
}
