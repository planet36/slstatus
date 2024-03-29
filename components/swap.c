/* See LICENSE file for copyright and license details. */
#include "../meter.h"
#include "../slstatus.h"
#include "../util.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* percentages will be clamped to 99 */
#define MAX_PCT_99

#define HIST_WIDTH 10
static_assert(HIST_WIDTH > 0, "HIST_WIDTH must be > 0");

#define METER_WIDTH 10
static_assert(METER_WIDTH > 0, "METER_WIDTH must be > 0");

static uintmax_t free_bytes, total_bytes, used_bytes;

#if defined(__linux__)
/*
 * https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/6/html/deployment_guide/s2-proc-meminfo
 * While the file shows kilobytes (kB; 1 kB equals 1000 B), it is actually
 * kibibytes (KiB; 1 KiB equals 1024 B). This imprecision in /proc/meminfo is
 * known, but is not corrected due to legacy concerns - programs rely on
 * /proc/meminfo to specify size with the "kB" string.
 */

	static int
	update_swap_info()
	{
		uintmax_t swaptotal, swapfree;

		if (pscanf("/proc/meminfo",
		              "MemTotal: %*s kB\n" // discard
		              "MemFree: %*s kB\n" // discard
		              "MemAvailable: %*s kB\n" // discard
		              "Buffers: %*s kB\n" // discard
		              "Cached: %*s kB\n" // discard
		              "SwapCached: %*s kB\n" // discard
		              "Active: %*s kB\n" // discard
		              "Inactive: %*s kB\n" // discard
		              "Active(anon): %*s kB\n" // discard
		              "Inactive(anon): %*s kB\n" // discard
		              "Active(file): %*s kB\n" // discard
		              "Inactive(file): %*s kB\n" // discard
		              "Unevictable: %*s kB\n" // discard
		              "Mlocked: %*s kB\n" // discard
		              "SwapTotal: %ju kB\n"
		              "SwapFree: %ju kB\n",
		              &swaptotal, &swapfree) != 2)
			return -1;

		free_bytes = swapfree * 1024;
		total_bytes = swaptotal * 1024;
		used_bytes = total_bytes - free_bytes;

		return 0;
	}
#elif defined(__OpenBSD__)
	#include <err.h>
	#include <stdlib.h>
	#include <sys/swap.h>
	#include <sys/types.h>
	#include <unistd.h>

	static int
	update_swap_info()
	{
		struct swapent *sep, *fsep;
		int rnswap, nswap, i;

		if ((nswap = swapctl(SWAP_NSWAP, 0, 0)) < 1) {
			warn("swaptctl 'SWAP_NSWAP'");
			return -1;
		}
		if (!(fsep = sep = calloc(nswap, sizeof(*sep)))) {
			warn("calloc 'nswap'");
			return -1;
		}
		if ((rnswap = swapctl(SWAP_STATS, (void *)sep, nswap)) < 0) {
			warn("swapctl 'SWAP_STATA'");
			free(fsep);
			return -1;
		}
		if (nswap != rnswap) {
			warnx("getstats: SWAP_STATS != SWAP_NSWAP");
			free(fsep);
			return -1;
		}

		total_bytes = 0;
		used_bytes = 0;

		for (i = 0; i < rnswap; i++) {
			total_bytes += sep->se_nblks >> 1;
			used_bytes += sep->se_inuse >> 1;
		}

		total_bytes *= 1024;
		used_bytes *= 1024;
		free_bytes = total_bytes - used_bytes;

		free(fsep);

		return 0;
	}
#elif defined(__FreeBSD__)
	#include <err.h>
	#include <fcntl.h>
	#include <kvm.h>
	#include <stdlib.h>
	#include <sys/types.h>
	#include <unistd.h>

	static int
	update_swap_info()
	{
		kvm_t *kd;
		struct kvm_swap swap_info[1];

		kd = kvm_openfiles(NULL, "/dev/null", NULL, 0, NULL);
		if (kd == NULL) {
			warn("kvm_openfiles '/dev/null'");
			return -1;
		}

		if (kvm_getswapinfo(kd, swap_info, LEN(swap_info), 0 /* Unused flags */) < 0) {
			warn("kvm_getswapinfo");
			(void)kvm_close(kd);
			return -1;
		}

		if (kvm_close(kd) < 0)
			return -1;

		total_bytes = swap_info[0].ksw_total * getpagesize();
		used_bytes = swap_info[0].ksw_used * getpagesize();
		free_bytes = total_bytes - used_bytes;

		return 0;
	}
#endif

const char *
swap_free([[maybe_unused]] const char *unused)
{
	if (update_swap_info() < 0)
		return NULL;

	return fmt_human_3(free_bytes, 1024);
}

const char *
swap_hist([[maybe_unused]] const char *unused)
{
	double used;
	static int initialized;
	size_t i;
	static wchar_t hist[HIST_WIDTH + 1];

	if (!initialized) {
		wmemset(hist, ' ', HIST_WIDTH);
		hist[HIST_WIDTH] = '\0';
		initialized = 1;
	}

	if (update_swap_info() < 0 || total_bytes == 0)
		return NULL;

	used = (double)used_bytes / total_bytes;

	for (i = 0; i < HIST_WIDTH - 1; ++i) {
		hist[i] = hist[i+1];
	}
	hist[i] = lower_blocks_1(used);

	return bprintf("%ls", hist);
}

const char *
swap_meter([[maybe_unused]] const char *unused)
{
	double used;
	wchar_t meter[METER_WIDTH + 1] = {'\0'};

	if (update_swap_info() < 0 || total_bytes == 0)
		return NULL;

	used = (double)used_bytes / total_bytes;

	left_blocks_meter(used, meter, METER_WIDTH);

	return bprintf("%ls", meter);
}

const char *
swap_perc([[maybe_unused]] const char *unused)
{
	double used;

	if (update_swap_info() < 0 || total_bytes == 0)
		return NULL;

	used = (double)used_bytes / total_bytes;

#ifdef MAX_PCT_99
	if (used > 0.99)
		used = 0.99;
#endif

	return bprintf("%.0f", 100 * used);
}

const char *
swap_total([[maybe_unused]] const char *unused)
{
	if (update_swap_info() < 0)
		return NULL;

	return fmt_human_3(total_bytes, 1024);
}

const char *
swap_used([[maybe_unused]] const char *unused)
{
	if (update_swap_info() < 0)
		return NULL;

	return fmt_human_3(used_bytes, 1024);
}
