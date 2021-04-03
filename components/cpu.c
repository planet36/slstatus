/* See LICENSE file for copyright and license details. */
#include "../util.h"

#include <stdio.h>
#include <string.h>

#if defined(__linux__)
	static int
	calc_freq(uintmax_t *freq)
	{
		uintmax_t tmp_freq;

		/* in kHz */
		if (pscanf("/sys/devices/system/cpu/cpu0/cpufreq/"
		           "scaling_cur_freq", "%ju", &tmp_freq) != 1) {
			return -1;
		}

		*freq = tmp_freq * 1000ULL; // KHz to Hz

		return 0;
	}

	static int
	calc_idle(uintmax_t *idle, uintmax_t *sum)
	{
		uintmax_t a[6];

		/* cpu user nice system idle iowait irq softirq */
		if (pscanf("/proc/stat", "%*s %ju %ju %ju %ju %*s %ju %ju",
		           &a[0], &a[1], &a[2], &a[3], &a[4], &a[5])
		    != 6) {
			return -1;
		}

		*idle = a[3];
		*sum = a[0] + a[1] + a[2] + a[3] + a[4] + a[5];

		return 0;
	}
#elif defined(__OpenBSD__)
	#include <sys/param.h>
	#include <sys/sched.h>
	#include <sys/sysctl.h>

	static int
	calc_freq(uintmax_t *freq)
	{
		int tmp_freq, mib[2];
		size_t size;

		mib[0] = CTL_HW;
		mib[1] = HW_CPUSPEED;

		size = sizeof(tmp_freq);

		/* in MHz */
		if (sysctl(mib, LEN(mib), &tmp_freq, &size, NULL, 0) < 0) {
			warn("sysctl 'HW_CPUSPEED':");
			return -1;
		}

		*freq = tmp_freq * 1000000ULL; // MHz to Hz

		return 0;
	}

	static int
	calc_idle(uintmax_t *idle, uintmax_t *sum)
	{
		int mib[2];
		uintmax_t a[CPUSTATES];
		size_t size;

		mib[0] = CTL_KERN;
		mib[1] = KERN_CPTIME;

		size = sizeof(a);

		if (sysctl(mib, LEN(mib), &a, &size, NULL, 0) < 0) {
			warn("sysctl 'KERN_CPTIME':");
			return -1;
		}

		*idle = a[CP_IDLE];
		*sum = a[CP_USER] + a[CP_NICE] + a[CP_SYS] + a[CP_INTR] + a[CP_IDLE];

		return 0;
	}
#elif defined(__FreeBSD__)
	#include <sys/param.h>
	#include <sys/sysctl.h>
	#include <devstat.h>

	static int
	calc_freq(uintmax_t *freq)
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

		*freq = tmp_freq * 1000000ULL; // MHz to Hz

		return 0;
	}

	static int
	calc_idle(uintmax_t *idle, uintmax_t *sum)
	{
		long a[CPUSTATES];
		size_t size;

		size = sizeof(a);

		if (sysctlbyname("kern.cp_time", &a, &size, NULL, 0) == -1
				|| !size) {
			warn("sysctlbyname 'kern.cp_time':");
			return -1;
		}

		*idle = a[CP_IDLE];
		*sum = a[CP_USER] + a[CP_NICE] + a[CP_SYS] + a[CP_INTR] + a[CP_IDLE];

		return 0;
	}
#endif

const char *
cpu_freq(void)
{
	uintmax_t freq; // Hz

	if (calc_freq(&freq) < 0) {
		return NULL;
	}

	return fmt_human(freq, 1000);
}

const char *
cpu_perc(void)
{
	static uintmax_t idle;
	uintmax_t oldidle = idle;
	static uintmax_t sum;
	uintmax_t oldsum = sum;
	double used;

	if (calc_idle(&idle, &sum) < 0 || oldidle == 0) {
		return NULL;
	}

	if (sum - oldsum == 0) {
		return NULL;
	}

	used = 1 - (double)(idle - oldidle) / (sum - oldsum);

	return bprintf("%.0f", 100 * used);
}
