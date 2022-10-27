/* See LICENSE file for copyright and license details. */
#include "../slstatus.h"
#include "../util.h"

#include <stddef.h>

#if defined(__linux__)
	#include <stdint.h>

	const char *
	temp(const char *file)
	{
		uintmax_t temp;

		if (pscanf(file, "%ju", &temp) != 1) {
			return NULL;
		}

		return bprintf("%0.f", temp / 1000.0);
	}
#elif defined(__OpenBSD__)
	#include <err.h>
	#include <stdio.h>
	#include <sys/time.h> /* before <sys/sensors.h> for struct timeval */
	#include <sys/sensors.h>
	#include <sys/sysctl.h>

	const char *
	temp(const char *unused)
	{
		int mib[5];
		size_t size;
		struct sensor temp;

		mib[0] = CTL_HW;
		mib[1] = HW_SENSORS;
		mib[2] = 0; /* cpu0 */
		mib[3] = SENSOR_TEMP;
		mib[4] = 0; /* temp0 */

		size = sizeof(temp);

		if (sysctl(mib, LEN(mib), &temp, &size, NULL, 0) < 0) {
			warn("sysctl 'SENSOR_TEMP'");
			return NULL;
		}

		/* kelvin to celsius */
		return bprintf("%.0f", (temp.value-273150000) / 1E6);
	}
#elif defined(__FreeBSD__)
	#include <stdio.h>
	#include <stdlib.h>
	#include <sys/sysctl.h>

	const char *
	temp(const char *zone)
	{
		char buf[256];
		int temp;
		size_t len;

		len = sizeof(temp);
		snprintf(buf, sizeof(buf), "hw.acpi.thermal.%s.temperature", zone);
		if (sysctlbyname(buf, &temp, &len, NULL, 0) < 0
				|| !len)
			return NULL;

		/* kelvin to decimal celcius */
		return bprintf("%.0f", (temp - 2731) / 10.0);
	}
#endif
