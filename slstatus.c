/* See LICENSE file for copyright and license details. */
#include "slstatus.h"
#include "util.h"

#include <err.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

struct component {
	const char *(*func)(const char *);
	const char *fmt;
	const char *args;
};

char buf[1024];
double delta_time = 0; // seconds
static volatile sig_atomic_t done;
static volatile sig_atomic_t reset_alarm = 1;
static Display *dpy;

#include "config.h"

static void
terminate(const int signo)
{
	if (signo == SIGALRM) {
	}
	else if (signo == SIGUSR1) {
		reset_alarm = 1;
	}
	else {
		done = 1;
	}
}

static void
usage(const char* argv0)
{
	printf("Usage: %s [-V] [-h] [-1] [-s]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int ch;
	const char *optstring = "+Vh1s";
	struct sigaction act;
	sigset_t newmask, oldmask, waitmask;
	const struct timeval interval_tv = msec_to_timeval(interval);
	const struct itimerval itv = {
	             .it_interval = interval_tv,
	             .it_value = interval_tv}; // If zero, the alarm is disabled.
	struct timespec start;
	double now_time, prev_time = 0;
	size_t i, len;
	int sflag, ret;
	char status[MAXLEN];
	const char *res;

	(void)setlocale(LC_CTYPE, "");

	sflag = 0;
	while ((ch = getopt(argc, argv, optstring)) != -1) {
		switch (ch) {
		case 'V':
			printf("%s " VERSION "\n", argv[0]);
			return 0;
			break;
		case 'h':
			usage(argv[0]);
			return 0;
			break;
		case '1':
			done = 1;
			/* FALLTHROUGH */
		case 's':
			sflag = 1;
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

	memset(&act, 0, sizeof(act));
	sigemptyset(&act.sa_mask);
	act.sa_handler = terminate;
	sigaction(SIGINT,  &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	act.sa_flags |= SA_RESTART;
	sigaction(SIGALRM, &act, NULL);
	sigaction(SIGUSR1, &act, NULL);

	if (!sflag && !(dpy = XOpenDisplay(NULL)))
		errx(EXIT_FAILURE, "XOpenDisplay: Failed to open display");

	sigemptyset(&waitmask);
	sigfillset(&newmask);
	sigprocmask(SIG_BLOCK, &newmask, &oldmask);

	do {
		if (reset_alarm) {
			if (setitimer(ITIMER_REAL, &itv, NULL) < 0) {
				err(EXIT_FAILURE, "setitimer");
			}
			reset_alarm = 0;
		}

		if (clock_gettime(CLOCK_MONOTONIC, &start) < 0)
			err(EXIT_FAILURE, "clock_gettime");

		now_time = timespec_to_sec(&start);
		delta_time = now_time - prev_time;
		prev_time = now_time;

		status[0] = '\0';
		for (i = len = 0; i < LEN(components); i++) {
			if (!(res = components[i].func(components[i].args)))
				res = unknown_str;

			if ((ret = esnprintf(status + len, sizeof(status) - len,
			                     components[i].fmt, res)) < 0)
				break;

			len += ret;
		}

		if (sflag) {
			(void)puts(status);
			(void)fflush(stdout);
			if (ferror(stdout))
				err(EXIT_FAILURE, "puts");
		} else {
			if (XStoreName(dpy, DefaultRootWindow(dpy), status) < 0)
				errx(EXIT_FAILURE, "XStoreName: Allocation failed");
			XFlush(dpy);
		}

		if (!done) {
			(void)sigsuspend(&waitmask);
		}
	} while (!done);

	sigprocmask(SIG_SETMASK, &oldmask, NULL);

	if (!sflag) {
		XStoreName(dpy, DefaultRootWindow(dpy), NULL);
		if (XCloseDisplay(dpy) < 0)
			errx(EXIT_FAILURE, "XCloseDisplay: Failed to close display");
	}

	return 0;
}
