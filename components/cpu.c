/* See LICENSE file for copyright and license details. */
#include "../util.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uintmax_t freq; // Hz
static double used;

#if defined(__linux__)

	static int
	update_freq(void)
	{
		uintmax_t tmp_freq;

		/* in kHz */
		if (pscanf("/sys/devices/system/cpu/cpu0/cpufreq/"
		           "scaling_cur_freq", "%ju", &tmp_freq) != 1) {
			return -1;
		}

		freq = tmp_freq * 1000ULL; // KHz to Hz

		return 0;
	}

	static int
	update_used(void)
	{
		static uintmax_t a[6];
		uintmax_t b[6], sum;

		memcpy(b, a, sizeof(b));
		/* cpu user nice system idle iowait irq softirq */
		if (pscanf("/proc/stat", "%*s %ju %ju %ju %ju %*s %ju %ju",
		           &a[0], &a[1], &a[2], &a[3], &a[4], &a[5])
		    != 6) {
			return -1;
		}
		if (b[0] == 0) {
			return -1;
		}

		sum = (a[0] + a[1] + a[2] + a[3] + a[4] + a[5]) -
		      (b[0] + b[1] + b[2] + b[3] + b[4] + b[5]);

		if (sum == 0) {
			return -1;
		}

		used = 1 - (double)(a[3] - b[3]) / (double)sum;

		return 0;
	}
#elif defined(__OpenBSD__)
	#include <sys/param.h>
	#include <sys/sched.h>
	#include <sys/sysctl.h>

	static int
	update_freq(void)
	{
		int tmp_freq, mib[2];
		size_t size;

		mib[0] = CTL_HW;
		mib[1] = HW_CPUSPEED;

		size = sizeof(tmp_freq);

		/* in MHz */
		if (sysctl(mib, 2, &tmp_freq, &size, NULL, 0) < 0) {
			warn("sysctl 'HW_CPUSPEED':");
			return -1;
		}

		freq = tmp_freq * 1000000ULL; // MHz to Hz

		return 0;
	}

	static int
	update_used(void)
	{
		int mib[2];
		static uintmax_t a[CPUSTATES];
		uintmax_t b[CPUSTATES], sum;
		size_t size;

		mib[0] = CTL_KERN;
		mib[1] = KERN_CPTIME;

		size = sizeof(a);

		memcpy(b, a, sizeof(b));
		if (sysctl(mib, 2, &a, &size, NULL, 0) < 0) {
			warn("sysctl 'KERN_CPTIME':");
			return -1;
		}
		if (b[0] == 0) {
			return -1;
		}

		sum = (a[CP_USER] + a[CP_NICE] + a[CP_SYS] + a[CP_INTR] + a[CP_IDLE]) -
		      (b[CP_USER] + b[CP_NICE] + b[CP_SYS] + b[CP_INTR] + b[CP_IDLE]);

		if (sum == 0) {
			return -1;
		}

		used = 1 - (double)(a[CP_IDLE] - b[CP_IDLE]) / (double)sum;

		return 0;
	}
#elif defined(__FreeBSD__)
	#include <sys/param.h>
	#include <sys/sysctl.h>
	#include <devstat.h>

	static int
	update_freq(void)
	{
		int tmp_freq;
		size_t size;

		size = sizeof(tmp_freq);
		/* in MHz */
		if (sysctlbyname("hw.clockrate", &tmp_freq, &size, NULL, 0) == -1
				|| !size) {
			warn("sysctlbyname 'hw.clockrate':");
			return -1;
		}

		freq = tmp_freq * 1000000ULL; // MHz to Hz

		return 0;
	}

	static int
	update_used(void)
	{
		static long a[CPUSTATES];
		long b[CPUSTATES], sum;
		size_t size;

		size = sizeof(a);
		memcpy(b, a, sizeof(b));
		if (sysctlbyname("kern.cp_time", &a, &size, NULL, 0) == -1
				|| !size) {
			warn("sysctlbyname 'kern.cp_time':");
			return -1;
		}
		if (b[0] == 0) {
			return -1;
		}

		sum = (a[CP_USER] + a[CP_NICE] + a[CP_SYS] + a[CP_INTR] + a[CP_IDLE]) -
		      (b[CP_USER] + b[CP_NICE] + b[CP_SYS] + b[CP_INTR] + b[CP_IDLE]);

		if (sum == 0) {
			return -1;
		}

		used = 1 - (double)(a[CP_IDLE] - b[CP_IDLE]) / (double)sum;

		return 0;
	}
#endif

const char *
cpu_freq(void)
{
	if (update_freq() < 0) {
		return NULL;
	}

	return fmt_human(freq, 1000);
}

const char *
cpu_perc(void)
{
	if (update_used() < 0) {
		return NULL;
	}

	return bprintf("%.0f", 100 * used);
}
