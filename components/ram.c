/* See LICENSE file for copyright and license details. */
#include "../util.h"

#include <stdio.h>

#if defined(__linux__)

/*
 * https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/6/html/deployment_guide/s2-proc-meminfo
 * While the file shows kilobytes (kB; 1 kB equals 1000 B), it is actually
 * kibibytes (KiB; 1 KiB equals 1024 B). This imprecision in /proc/meminfo is
 * known, but is not corrected due to legacy concerns - programs rely on
 * /proc/meminfo to specify size with the "kB" string.
 */

	static uintmax_t memtotal, memfree, memavailable, buffers, cached,
	                 sreclaimable;

	static int
	update_mem_info(void)
	{
		return pscanf("/proc/meminfo",
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
		              &sreclaimable) != 6;
	}

	const char *
	ram_free(void)
	{
		if (update_mem_info()) {
			return NULL;
		}

		return fmt_human_3(memavailable * 1024, 1024);
	}

	const char *
	ram_perc(void)
	{
		uintmax_t used;

		if (update_mem_info() || memtotal == 0) {
			return NULL;
		}

		used = memtotal - memfree - (buffers + cached + sreclaimable);
		return bprintf("%.0f", 100.0 * used / memtotal);
	}

	const char *
	ram_total(void)
	{
		if (update_mem_info()) {
			return NULL;
		}

		return fmt_human_3(memtotal * 1024, 1024);
	}

	const char *
	ram_used(void)
	{
		uintmax_t used;

		if (update_mem_info()) {
			return NULL;
		}

		used = memtotal - memfree - (buffers + cached + sreclaimable);
		return fmt_human_3(used * 1024, 1024);
	}
#elif defined(__OpenBSD__)
	#include <stdlib.h>
	#include <sys/sysctl.h>
	#include <sys/types.h>
	#include <unistd.h>

	#define LOG1024 10
	#define pagetok(size, pageshift) (size_t)((size) << ((pageshift) - LOG1024))

	static int
	load_uvmexp(struct uvmexp *uvmexp)
	{
		int uvmexp_mib[2] = {CTL_VM, VM_UVMEXP};
		size_t size;

		size = sizeof(*uvmexp);

		return sysctl(uvmexp_mib, LEN(uvmexp_mib), uvmexp, &size, NULL, 0);
	}

	const char *
	ram_free(void)
	{
		struct uvmexp uvmexp;
		int free_pages;

		if (load_uvmexp(&uvmexp) < 0) {
			return NULL;
		}

		free_pages = uvmexp.npages - uvmexp.active;
		return fmt_human(pagetok(free_pages, uvmexp.pageshift) *
		                 1024, 1024);
	}

	const char *
	ram_perc(void)
	{
		struct uvmexp uvmexp;

		if (load_uvmexp(&uvmexp) < 0) {
			return NULL;
		}

		return bprintf("%.0f", 100.0 * uvmexp.active / uvmexp.npages);
	}

	const char *
	ram_total(void)
	{
		struct uvmexp uvmexp;

		if (load_uvmexp(&uvmexp) < 0) {
			return NULL;
		}

		return fmt_human(pagetok(uvmexp.npages,
		                         uvmexp.pageshift) * 1024,
		                 1024);
	}

	const char *
	ram_used(void)
	{
		struct uvmexp uvmexp;

		if (load_uvmexp(&uvmexp) < 0) {
			return NULL;
		}

		return fmt_human(pagetok(uvmexp.active,
		                         uvmexp.pageshift) * 1024,
		                 1024);
	}
#elif defined(__FreeBSD__)
	#include <sys/sysctl.h>
	#include <sys/vmmeter.h>
	#include <unistd.h>
	#include <vm/vm_param.h>

	const char *
	ram_free(void)
	{
		struct vmtotal vm_stats;
		int mib[2] = {CTL_VM, VM_TOTAL};
		size_t len;

		len = sizeof(vm_stats);
		if (sysctl(mib, LEN(mib), &vm_stats, &len, NULL, 0) < 0 || !len)
			return NULL;

		return fmt_human(vm_stats.t_free * getpagesize(), 1024);
	}

	const char *
	ram_total(void)
	{
		unsigned int npages;
		size_t len;

		len = sizeof(npages);
		if (sysctlbyname("vm.stats.vm.v_page_count",
		                 &npages, &len, NULL, 0) < 0 || !len)
			return NULL;

		return fmt_human(npages * getpagesize(), 1024);
	}

	const char *
	ram_perc(void)
	{
		unsigned int npages;
		unsigned int active;
		size_t len;

		len = sizeof(npages);
		if (sysctlbyname("vm.stats.vm.v_page_count",
		                 &npages, &len, NULL, 0) < 0 || !len)
			return NULL;

		if (sysctlbyname("vm.stats.vm.v_active_count",
		                 &active, &len, NULL, 0) < 0 || !len)
			return NULL;

		return bprintf("%.0f", 100.0 * active / npages);
	}

	const char *
	ram_used(void)
	{
		unsigned int active;
		size_t len;

		len = sizeof(active);
		if (sysctlbyname("vm.stats.vm.v_active_count",
		                 &active, &len, NULL, 0) < 0 || !len)
			return NULL;

		return fmt_human(active * getpagesize(), 1024);
	}
#endif
