/* See LICENSE file for copyright and license details. */
#include "../slstatus.h"
#include "../util.h"

#include <err.h>
#include <stdio.h>
#include <unistd.h>

const char *
hostname(void)
{
	if (gethostname(buf, sizeof(buf)) < 0) {
		warn("gethostbyname");
		return NULL;
	}

	return buf;
}
