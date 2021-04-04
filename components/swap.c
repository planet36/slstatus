/* See LICENSE file for copyright and license details. */
#include "../util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)

/*
 * https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/6/html/deployment_guide/s2-proc-meminfo
 * While the file shows kilobytes (kB; 1 kB equals 1000 B), it is actually
 * kibibytes (KiB; 1 KiB equals 1024 B). This imprecision in /proc/meminfo is
 * known, but is not corrected due to legacy concerns - programs rely on
 * /proc/meminfo to specify size with the "kB" string.
 */

	static uintmax_t swaptotal, swapfree;

	static int
	update_swap_info()
	{
		return pscanf("/proc/meminfo",
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
		              &swaptotal, &swapfree) != 2;
	}

	const char *
	swap_free(void)
	{
		if (update_swap_info()) {
			return NULL;
		}

		return fmt_human_3(swapfree * 1024, 1024);
	}

	const char *
	swap_perc(void)
	{
		uintmax_t used;

		if (update_swap_info() || swaptotal == 0) {
			return NULL;
		}

		used = (swaptotal - swapfree);
		return bprintf("%.0f", 100.0 * used / swaptotal);
	}

	const char *
	swap_total(void)
	{
		if (update_swap_info()) {
			return NULL;
		}

		return fmt_human_3(swaptotal * 1024, 1024);
	}

	const char *
	swap_used(void)
	{
		if (update_swap_info()) {
			return NULL;
		}

		return fmt_human_3((swaptotal - swapfree) * 1024, 1024);
	}
#elif defined(__OpenBSD__)
	#include <stdlib.h>
	#include <sys/swap.h>
	#include <sys/types.h>
	#include <unistd.h>

	static int
	getstats(int *total, int *used)
	{
		struct swapent *sep, *fsep;
		int rnswap, nswap, i;

		if ((nswap = swapctl(SWAP_NSWAP, 0, 0)) < 1) {
			warn("swaptctl 'SWAP_NSWAP':");
			return 1;
		}
		if (!(fsep = sep = calloc(nswap, sizeof(*sep)))) {
			warn("calloc 'nswap':");
			return 1;
		}
		if ((rnswap = swapctl(SWAP_STATS, (void *)sep, nswap)) < 0) {
			warn("swapctl 'SWAP_STATA':");
			return 1;
		}
		if (nswap != rnswap) {
			warn("getstats: SWAP_STATS != SWAP_NSWAP");
			return 1;
		}

		*total = 0;
		*used = 0;

		for (i = 0; i < rnswap; i++) {
			*total += sep->se_nblks >> 1;
			*used += sep->se_inuse >> 1;
		}

		free(fsep);

		return 0;
	}

	const char *
	swap_free(void)
	{
		int total, used;

		if (getstats(&total, &used)) {
			return NULL;
		}

		return fmt_human((total - used) * 1024, 1024);
	}

	const char *
	swap_perc(void)
	{
		int total, used;

		if (getstats(&total, &used)) {
			return NULL;
		}

		if (total == 0) {
			return NULL;
		}

		return bprintf("%.0f", 100.0 * used / total);
	}

	const char *
	swap_total(void)
	{
		int total, used;

		if (getstats(&total, &used)) {
			return NULL;
		}

		return fmt_human(total * 1024, 1024);
	}

	const char *
	swap_used(void)
	{
		int total, used;

		if (getstats(&total, &used)) {
			return NULL;
		}

		return fmt_human(used * 1024, 1024);
	}
#elif defined(__FreeBSD__)
	#include <stdlib.h>
	#include <sys/types.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <kvm.h>

	static int
	getswapinfo(struct kvm_swap *swap_info, size_t size)
	{
		kvm_t *kd;

		kd = kvm_openfiles(NULL, "/dev/null", NULL, 0, NULL);
		if (kd == NULL) {
			warn("kvm_openfiles '/dev/null':");
			return 0;
		}

		if (kvm_getswapinfo(kd, swap_info, size, 0 /* Unused flags */) == -1) {
			warn("kvm_getswapinfo:");
			kvm_close(kd);
			return 0;
		}

		kvm_close(kd);
		return 1;
	}

	const char *
	swap_free(void)
	{
		struct kvm_swap swap_info[1];
		long used, total;

		if (!getswapinfo(swap_info, 1))
			return NULL;

		total = swap_info[0].ksw_total;
		used = swap_info[0].ksw_used;

		return fmt_human((total - used) * getpagesize(), 1024);
	}

	const char *
	swap_perc(void)
	{
		struct kvm_swap swap_info[1];
		long used, total;

		if (!getswapinfo(swap_info, 1))
			return NULL;

		total = swap_info[0].ksw_total;
		used = swap_info[0].ksw_used;

		return bprintf("%.0f", 100.0 * used / total);
	}

	const char *
	swap_total(void)
	{
		struct kvm_swap swap_info[1];
		long total;

		if (!getswapinfo(swap_info, 1))
			return NULL;

		total = swap_info[0].ksw_total;

		return fmt_human(total * getpagesize(), 1024);
	}

	const char *
	swap_used(void)
	{
		struct kvm_swap swap_info[1];
		long used;

		if (!getswapinfo(swap_info, 1))
			return NULL;

		used = swap_info[0].ksw_used;

		return fmt_human(used * getpagesize(), 1024);
	}
#endif
