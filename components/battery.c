/* See LICENSE file for copyright and license details. */
#include "../meter.h"
#include "../util.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* percentages will be clamped to 99 */
//#define MAX_PCT_99

#define METER_WIDTH 10
static_assert(METER_WIDTH > 0, "METER_WIDTH must be > 0");

#if defined(__linux__)
/*
 * https://www.kernel.org/doc/html/latest/power/power_supply_class.html
 */
	#include <limits.h>
	#include <stdint.h>
	#include <unistd.h>

	static const char *
	pick(const char *bat, const char *f1, const char *f2, char *path,
	     size_t length)
	{
		if (esnprintf(path, length, f1, bat) > 0 &&
		    access(path, R_OK) == 0) {
			return f1;
		}

		if (esnprintf(path, length, f2, bat) > 0 &&
		    access(path, R_OK) == 0) {
			return f2;
		}

		return NULL;
	}

	const char *
	battery_meter(const char *bat)
	{
		int cap_perc;
		char path[PATH_MAX];
		wchar_t meter[METER_WIDTH + 1] = {'\0'};

		if (esnprintf(path, sizeof(path),
		              "/sys/class/power_supply/%s/capacity", bat) < 0) {
			return NULL;
		}
		if (pscanf(path, "%d", &cap_perc) != 1) {
			return NULL;
		}

		left_blocks_meter(cap_perc / 100.0, meter, METER_WIDTH);

		return bprintf("%ls", meter);
	}

	const char *
	battery_perc(const char *bat)
	{
		int cap_perc;
		char path[PATH_MAX];

		if (esnprintf(path, sizeof(path),
		              "/sys/class/power_supply/%s/capacity", bat) < 0) {
			return NULL;
		}
		if (pscanf(path, "%d", &cap_perc) != 1) {
			return NULL;
		}

#ifdef MAX_PCT_99
		if (cap_perc > 99)
			cap_perc = 99;
#endif

		return bprintf("%d", cap_perc);
	}

	const char *
	battery_state(const char *bat)
	{
		static struct {
			char *state;
			char *symbol;
		} map[] = {
			{ "Charging",    "+" },
			{ "Discharging", "-" },
			{ "Full",        "o" },
			{ "Not charging", "o" },
		};
		size_t i;
		char path[PATH_MAX], state[12];

		if (esnprintf(path, sizeof(path),
		              "/sys/class/power_supply/%s/status", bat) < 0) {
			return NULL;
		}
		if (pscanf(path, "%12[a-zA-Z ]", state) != 1) {
			return NULL;
		}

		for (i = 0; i < LEN(map); i++) {
			if (!strcmp(map[i].state, state)) {
				break;
			}
		}
		return (i == LEN(map)) ? "?" : map[i].symbol;
	}

	const char *
	battery_remaining(const char *bat)
	{
		uintmax_t charge_now, current_now, m, h;
		double timeleft;
		char path[PATH_MAX], state[12];

		if (esnprintf(path, sizeof(path),
		              "/sys/class/power_supply/%s/status", bat) < 0) {
			return NULL;
		}
		if (pscanf(path, "%12[a-zA-Z ]", state) != 1) {
			return NULL;
		}

		if (!pick(bat, "/sys/class/power_supply/%s/charge_now",
		          "/sys/class/power_supply/%s/energy_now", path,
		          sizeof(path)) ||
		    pscanf(path, "%ju", &charge_now) != 1) {
			return NULL;
		}

		if (!strcmp(state, "Discharging")) {
			if (!pick(bat, "/sys/class/power_supply/%s/current_now",
			          "/sys/class/power_supply/%s/power_now", path,
			          sizeof(path)) ||
			    pscanf(path, "%ju", &current_now) != 1) {
				return NULL;
			}

			if (current_now == 0) {
				return NULL;
			}

			timeleft = (double)charge_now / current_now;
			h = timeleft;
			m = (uintmax_t)(timeleft - h) * 60;

			return bprintf("%juh %02jum", h, m);
		}

		return "";
	}
#elif defined(__OpenBSD__)
	#include <err.h>
	#include <fcntl.h>
	#include <machine/apmvar.h>
	#include <sys/ioctl.h>
	#include <unistd.h>

	static int
	load_apm_power_info(struct apm_power_info *apm_info)
	{
		int fd;

		fd = open("/dev/apm", O_RDONLY);
		if (fd < 0) {
			warn("open '/dev/apm'");
			return -1;
		}

		memset(apm_info, 0, sizeof(struct apm_power_info));
		if (ioctl(fd, APM_IOC_GETPOWER, apm_info) < 0) {
			warn("ioctl 'APM_IOC_GETPOWER'");
			close(fd);
			return -1;
		}

		close(fd);
		return 0;
	}

	const char *
	battery_meter(const char *unused)
	{
		struct apm_power_info apm_info;
		wchar_t meter[METER_WIDTH + 1] = {'\0'};

		if (load_apm_power_info(&apm_info) < 0) {
			return NULL;
		}

		left_blocks_meter(apm_info.battery_life / 100.0, meter, METER_WIDTH);

		return bprintf("%ls", meter);
	}

	const char *
	battery_perc(const char *unused)
	{
		int cap_perc;
		struct apm_power_info apm_info;

		if (load_apm_power_info(&apm_info) < 0) {
			return NULL;
		}

		cap_perc = apm_info.battery_life;

#ifdef MAX_PCT_99
		if (cap_perc > 99)
			cap_perc = 99;
#endif

		return bprintf("%d", cap_perc);
	}

	const char *
	battery_state(const char *unused)
	{
		struct {
			unsigned int state;
			char *symbol;
		} map[] = {
			{ APM_AC_ON,      "+" },
			{ APM_AC_OFF,     "-" },
		};
		struct apm_power_info apm_info;
		size_t i;

		if (load_apm_power_info(&apm_info)) {
			return NULL;
		}

		for (i = 0; i < LEN(map); i++) {
			if (map[i].state == apm_info.ac_state) {
				break;
			}
		}
		return (i == LEN(map)) ? "?" : map[i].symbol;
	}

	const char *
	battery_remaining(const char *unused)
	{
		struct apm_power_info apm_info;

		if (load_apm_power_info(&apm_info) < 0) {
			return NULL;
		}

		if (apm_info.ac_state != APM_AC_ON) {
			return bprintf("%uh %02um",
			               apm_info.minutes_left / 60,
			               apm_info.minutes_left % 60);
		} else {
			return "";
		}
	}
#elif defined(__FreeBSD__)
	#include <sys/sysctl.h>

	const char *
	battery_meter(const char *unused)
	{
		int cap_perc;
		size_t len;
		wchar_t meter[METER_WIDTH + 1] = {'\0'};

		len = sizeof(cap_perc);
		if (sysctlbyname("hw.acpi.battery.life", &cap_perc, &len, NULL, 0) < 0
				|| !len)
			return NULL;

		left_blocks_meter(cap_perc / 100.0, meter, METER_WIDTH);

		return bprintf("%ls", meter);
	}

	const char *
	battery_perc(const char *unused)
	{
		int cap_perc;
		size_t len;

		len = sizeof(cap_perc);
		if (sysctlbyname("hw.acpi.battery.life", &cap_perc, &len, NULL, 0) < 0
				|| !len)
			return NULL;

#ifdef MAX_PCT_99
		if (cap_perc > 99)
			cap_perc = 99;
#endif

		return bprintf("%d", cap_perc);
	}

	const char *
	battery_state(const char *unused)
	{
		int state;
		size_t len;

		len = sizeof(state);
		if (sysctlbyname("hw.acpi.battery.state", &state, &len, NULL, 0) < 0
				|| !len)
			return NULL;

		switch (state) {
			case 0:
			case 2:
				return "+";
			case 1:
				return "-";
			default:
				return "?";
		}
	}

	const char *
	battery_remaining(const char *unused)
	{
		int rem;
		size_t len;

		len = sizeof(rem);
		if (sysctlbyname("hw.acpi.battery.time", &rem, &len, NULL, 0) < 0
				|| !len
				|| rem == -1)
			return NULL;

		return bprintf("%uh %02um", rem / 60, rem % 60);
	}
#endif
