/* See LICENSE file for copyright and license details. */
#include "../slstatus.h"
#include "../util.h"

#include <limits.h>
#include <stdio.h>

extern double delta_time; // seconds

#if defined(__linux__)
	#define NET_RX_BYTES "/sys/class/net/%s/statistics/rx_bytes"
	#define NET_TX_BYTES "/sys/class/net/%s/statistics/tx_bytes"

	static int
	calc_rxbytes(const char *interface, uintmax_t *rxbytes)
	{
		char path[PATH_MAX];

		if (esnprintf(path, sizeof(path), NET_RX_BYTES, interface) < 0)
			return -1;
		if (pscanf(path, "%ju", rxbytes) != 1)
			return -1;

		return 0;
	}

	static int
	calc_txbytes(const char *interface, uintmax_t *txbytes)
	{
		char path[PATH_MAX];

		if (esnprintf(path, sizeof(path), NET_TX_BYTES, interface) < 0)
			return -1;
		if (pscanf(path, "%ju", txbytes) != 1)
			return -1;

		return 0;
	}
#elif defined(__OpenBSD__) | defined(__FreeBSD__)
	#include <err.h>
	#include <ifaddrs.h>
	#include <net/if.h>
	#include <string.h>
	#include <sys/types.h>
	#include <sys/socket.h>

	static int
	calc_rxbytes(const char *interface, uintmax_t *rxbytes)
	{
		struct ifaddrs *ifal, *ifa;
		struct if_data *ifd;
		int if_ok = 0;

		if (getifaddrs(&ifal) < 0) {
			warnx("getifaddrs failed");
			return -1;
		}
		*rxbytes = 0;
		for (ifa = ifal; ifa; ifa = ifa->ifa_next)
			if (!strcmp(ifa->ifa_name, interface) &&
			   (ifd = (struct if_data *)ifa->ifa_data))
				*rxbytes += ifd->ifi_ibytes, if_ok = 1;

		freeifaddrs(ifal);
		if (!if_ok) {
			warnx("reading 'if_data' failed");
			return -1;
		}

		return 0;
	}

	static int
	calc_txbytes(const char *interface, uintmax_t *txbytes)
	{
		struct ifaddrs *ifal, *ifa;
		struct if_data *ifd;
		int if_ok = 0;

		if (getifaddrs(&ifal) < 0) {
			warnx("getifaddrs failed");
			return -1;
		}
		*txbytes = 0;
		for (ifa = ifal; ifa; ifa = ifa->ifa_next)
			if (!strcmp(ifa->ifa_name, interface) &&
			   (ifd = (struct if_data *)ifa->ifa_data))
				*txbytes += ifd->ifi_obytes, if_ok = 1;

		freeifaddrs(ifal);
		if (!if_ok) {
			warnx("reading 'if_data' failed");
			return -1;
		}

		return 0;
	}
#endif

const char *
netspeed_rx(const char *interface)
{
	static uintmax_t rxbytes;
	const uintmax_t oldrxbytes = rxbytes;

	if (calc_rxbytes(interface, &rxbytes) < 0 || oldrxbytes == 0)
		return NULL;

	return fmt_human_3((rxbytes - oldrxbytes) / delta_time, 1024);
}

const char *
netspeed_tx(const char *interface)
{
	static uintmax_t txbytes;
	const uintmax_t oldtxbytes = txbytes;

	if (calc_txbytes(interface, &txbytes) < 0 || oldtxbytes == 0)
		return NULL;

	return fmt_human_3((txbytes - oldtxbytes) / delta_time, 1024);
}
