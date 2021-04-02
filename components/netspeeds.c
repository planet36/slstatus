/* See LICENSE file for copyright and license details. */
#include "../util.h"

#include <limits.h>
#include <stdio.h>

extern double delta_time; // seconds

static uintmax_t rxbytes, oldrxbytes;
static uintmax_t txbytes, oldtxbytes;

#if defined(__linux__)
	static int
	update_rx(const char *interface)
	{
		char path[PATH_MAX];

		oldrxbytes = rxbytes;

		if (esnprintf(path, sizeof(path),
		              "/sys/class/net/%s/statistics/rx_bytes",
		              interface) < 0) {
			return -1;
		}
		if (pscanf(path, "%ju", &rxbytes) != 1) {
			return -1;
		}
		if (oldrxbytes == 0) {
			return -1;
		}

		return 0;
	}

	static int
	update_tx(const char *interface)
	{
		char path[PATH_MAX];

		oldtxbytes = txbytes;

		if (esnprintf(path, sizeof(path),
		              "/sys/class/net/%s/statistics/tx_bytes",
		              interface) < 0) {
			return -1;
		}
		if (pscanf(path, "%ju", &txbytes) != 1) {
			return -1;
		}
		if (oldtxbytes == 0) {
			return -1;
		}

		return 0;
	}
#elif defined(__OpenBSD__) | defined(__FreeBSD__)
	#include <string.h>
	#include <ifaddrs.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <net/if.h>

	static int
	update_rx(const char *interface)
	{
		struct ifaddrs *ifal, *ifa;
		struct if_data *ifd;
		int if_ok = 0;

		oldrxbytes = rxbytes;

		if (getifaddrs(&ifal) == -1) {
			warn("getifaddrs failed");
			return -1;
		}
		rxbytes = 0;
		for (ifa = ifal; ifa; ifa = ifa->ifa_next) {
			if (!strcmp(ifa->ifa_name, interface) &&
			   (ifd = (struct if_data *)ifa->ifa_data)) {
				rxbytes += ifd->ifi_ibytes, if_ok = 1;
			}
		}
		freeifaddrs(ifal);
		if (!if_ok) {
			warn("reading 'if_data' failed");
			return -1;
		}
		if (oldrxbytes == 0) {
			return -1;
		}

		return 0;
	}

	static int
	update_tx(const char *interface)
	{
		struct ifaddrs *ifal, *ifa;
		struct if_data *ifd;
		int if_ok = 0;

		oldtxbytes = txbytes;

		if (getifaddrs(&ifal) == -1) {
			warn("getifaddrs failed");
			return -1;
		}
		txbytes = 0;
		for (ifa = ifal; ifa; ifa = ifa->ifa_next) {
			if (!strcmp(ifa->ifa_name, interface) &&
			   (ifd = (struct if_data *)ifa->ifa_data)) {
				txbytes += ifd->ifi_obytes, if_ok = 1;
			}
		}
		freeifaddrs(ifal);
		if (!if_ok) {
			warn("reading 'if_data' failed");
			return -1;
		}
		if (oldtxbytes == 0) {
			return -1;
		}

		return 0;
	}
#endif

const char *
netspeed_rx(const char *interface)
{
	if (update_rx(interface) < 0) {
		return NULL;
	}

	return fmt_human_3((rxbytes - oldrxbytes) / delta_time, 1024);
}

const char *
netspeed_tx(const char *interface)
{
	if (update_tx(interface) < 0) {
		return NULL;
	}

	return fmt_human_3((txbytes - oldtxbytes) / delta_time, 1024);
}
