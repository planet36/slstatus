/* See LICENSE file for copyright and license details. */
#include "slstatus.h"
#include "util.h"

#include <X11/Xlib.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
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
	printf("usage: %s [-h] [-1] [-s] [-v]\n", argv0);
}

int
main(int argc, char *argv[])
{
	int ch;
	const char *optstring = "+h1sv";
	struct sigaction act;
	sigset_t newmask, oldmask, waitmask;
	const struct timeval interval_tv = {
	             .tv_sec = interval / 1000U,
	             .tv_usec = (interval % 1000U) * 1000U};
	const struct itimerval itv = {
	             .it_interval = interval_tv,
	             .it_value = interval_tv}; // If zero, the alarm is disabled.
	struct timespec start;
	double now_time, prev_time = 0;
	size_t i, len;
	int sflag, ret;
	char status[MAXLEN];
	const char *res;

	setlocale(LC_CTYPE, "");

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

	memset(&act, 0, sizeof(act));
	sigemptyset(&act.sa_mask);
	act.sa_handler = terminate;
	sigaction(SIGINT,  &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	act.sa_flags |= SA_RESTART;
	sigaction(SIGALRM, &act, NULL);
	sigaction(SIGUSR1, &act, NULL);

	if (!sflag && !(dpy = XOpenDisplay(NULL))) {
		die("XOpenDisplay: Failed to open display");
	}

	sigemptyset(&waitmask);
	sigfillset(&newmask);
	sigprocmask(SIG_BLOCK, &newmask, &oldmask);

	do {
		if (reset_alarm) {
			if (setitimer(ITIMER_REAL, &itv, NULL) < 0) {
				die("setitimer:");
			}
			reset_alarm = 0;
		}

		if (clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
			die("clock_gettime:");
		}
		now_time = timespec_to_double(&start);
		delta_time = now_time - prev_time;
		prev_time = now_time;

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
			(void)sigsuspend(&waitmask);
		}
	} while (!done);

	sigprocmask(SIG_SETMASK, &oldmask, NULL);

	if (!sflag) {
		XStoreName(dpy, DefaultRootWindow(dpy), NULL);
		if (XCloseDisplay(dpy) < 0) {
			die("XCloseDisplay: Failed to close display");
		}
	}

	return 0;
}
