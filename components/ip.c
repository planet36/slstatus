/* See LICENSE file for copyright and license details. */
#include "../slstatus.h"
#include "../util.h"

#include <err.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#if defined(__OpenBSD__)
	#include <sys/socket.h>
	#include <sys/types.h>
#elif defined(__FreeBSD__)
	#include <netinet/in.h>
	#include <sys/socket.h>
#endif

static const char *
ip(const char *interface, unsigned short sa_family)
{
	struct ifaddrs *ifaddr, *ifa;
	int s;
	char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) < 0) {
		warn("getifaddrs");
		return NULL;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr)
			continue;

		s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6),
		                host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if (!strcmp(ifa->ifa_name, interface) &&
		    (ifa->ifa_addr->sa_family == sa_family)) {
			freeifaddrs(ifaddr);
			if (s != 0) {
				warnx("getnameinfo: %s", gai_strerror(s));
				return NULL;
			}
			return bprintf("%s", host);
		}
	}

	freeifaddrs(ifaddr);

	return NULL;
}

const char *
ipv4(const char *interface)
{
	return ip(interface, AF_INET);
}

const char *
ipv6(const char *interface)
{
	return ip(interface, AF_INET6);
}

const char *
up(const char *interface)
{
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) < 0) {
		warn("getifaddrs");
		return NULL;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr)
			continue;

		if (!strcmp(ifa->ifa_name, interface)) {
			freeifaddrs(ifaddr);
			return ifa->ifa_flags & IFF_UP ? "up" : "down";
		}
	}

	freeifaddrs(ifaddr);

	return NULL;
}
