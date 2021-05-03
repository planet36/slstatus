/* See LICENSE file for copyright and license details. */
#include "../meter.h"
#include "../util.h"

#include <assert.h>
#include <stdio.h>

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
	update_mem_info(void)
	{
		uintmax_t memtotal, memfree, memavailable, buffers, cached,
		          sreclaimable;

		if (pscanf("/proc/meminfo",
		              "MemTotal: %ju kB\n"
		              "MemFree: %ju kB\n"
		              "MemAvailable: %ju kB\n"
		              "Buffers: %ju kB\n"
		              "Cached: %ju kB\n"
		              "SwapCached: %*s kB\n" // discard
		              "Active: %*s kB\n" // discard
		              "Inactive: %*s kB\n" // discard
		              "Active(anon): %*s kB\n" // discard
		              "Inactive(anon): %*s kB\n" // discard
		              "Active(file): %*s kB\n" // discard
		              "Inactive(file): %*s kB\n" // discard
		              "Unevictable: %*s kB\n" // discard
		              "Mlocked: %*s kB\n" // discard
		              "SwapTotal: %*s kB\n" // discard
		              "SwapFree: %*s kB\n" // discard
		              "Dirty: %*s kB\n" // discard
		              "Writeback: %*s kB\n" // discard
		              "AnonPages: %*s kB\n" // discard
		              "Mapped: %*s kB\n" // discard
		              "Shmem: %*s kB\n" // discard
		              "KReclaimable: %*s kB\n" // discard
		              "Slab: %*s kB\n" // discard
		              "SReclaimable: %ju kB\n",
		              &memtotal, &memfree, &memavailable, &buffers, &cached,
		              &sreclaimable) != 6) {
			return -1;
		}

		free_bytes = memavailable * 1024;
		total_bytes = memtotal * 1024;
		used_bytes = (memtotal - memfree - (buffers + cached + sreclaimable)) * 1024;

		return 0;
	}
#elif defined(__OpenBSD__)
	#include <stdlib.h>
	#include <sys/sysctl.h>
	#include <sys/types.h>
	#include <unistd.h>

	#define LOG1024 10
	#define pagetok(size, pageshift) (size_t)((size) << ((pageshift) - LOG1024))

	static int
	update_mem_info(void)
	{
		struct uvmexp uvmexp;
		int uvmexp_mib[2] = {CTL_VM, VM_UVMEXP};
		size_t size;

		size = sizeof(uvmexp);

		if (sysctl(uvmexp_mib, LEN(uvmexp_mib), &uvmexp, &size, NULL, 0) < 0) {
			return NULL;
		}

		// TODO: maybe add this: uvmexp.inactive
		free_bytes = pagetok(uvmexp.free, uvmexp.pageshift) * 1024;
		total_bytes = pagetok(uvmexp.npages, uvmexp.pageshift) * 1024;
		// TODO: maybe add this: uvmexp.wired
		used_bytes = pagetok(uvmexp.active, uvmexp.pageshift) * 1024;

		return 0;
	}
#elif defined(__FreeBSD__)
	#include <sys/sysctl.h>
	#include <sys/vmmeter.h>
	#include <unistd.h>
	#include <vm/vm_param.h>

	static int
	update_mem_info(void)
	{
		unsigned int free_pages, total_pages, active_pages;
		size_t len;

		len = sizeof(free_pages);
		if (sysctlbyname("vm.stats.vm.v_free_count",
		                 &free_pages, &len, NULL, 0) < 0 || !len)
			return -1;

		len = sizeof(total_pages);
		if (sysctlbyname("vm.stats.vm.v_page_count",
		                 &total_pages, &len, NULL, 0) < 0 || !len)
			return -1;

		len = sizeof(active_pages);
		if (sysctlbyname("vm.stats.vm.v_active_count",
		                 &active_pages, &len, NULL, 0) < 0 || !len)
			return -1;

		// TODO: maybe add these: v_cache_count, v_inactive_count
		free_bytes = free_pages* getpagesize();
		total_bytes = total_pages * getpagesize();
		// TODO: maybe add this: v_wire_count
		used_bytes = active_pages * getpagesize();

		return 0;
	}
#endif

const char *
ram_free(void)
{
	if (update_mem_info() < 0) {
		return NULL;
	}

	return fmt_human_3(free_bytes, 1024);
}

const char *
ram_hist(void)
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

	if (update_mem_info() < 0 || total_bytes == 0) {
		return NULL;
	}

	used = (double)used_bytes / total_bytes;

	for (i = 0; i < HIST_WIDTH - 1; ++i) {
		hist[i] = hist[i+1];
	}
	hist[i] = lower_blocks_1(used);

	return bprintf("%ls", hist);
}

const char *
ram_meter(void)
{
	double used;
	wchar_t meter[METER_WIDTH + 1] = {'\0'};

	if (update_mem_info() < 0 || total_bytes == 0) {
		return NULL;
	}

	used = (double)used_bytes / total_bytes;

	left_blocks_meter(used, meter, METER_WIDTH);

	return bprintf("%ls", meter);
}

const char *
ram_perc(void)
{
	double used;

	if (update_mem_info() < 0 || total_bytes == 0) {
		return NULL;
	}

	used = (double)used_bytes / total_bytes;

#ifdef MAX_PCT_99
	if (used > 0.99)
		used = 0.99;
#endif

	return bprintf("%.0f", 100 * used);
}

const char *
ram_total(void)
{
	if (update_mem_info() < 0) {
		return NULL;
	}

	return fmt_human_3(total_bytes, 1024);
}

const char *
ram_used(void)
{
	if (update_mem_info() < 0) {
		return NULL;
	}

	return fmt_human_3(used_bytes, 1024);
}
