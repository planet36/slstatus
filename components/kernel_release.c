/* See LICENSE file for copyright and license details. */
#include "../slstatus.h"
#include "../util.h"

#include <err.h>
#include <stdio.h>
#include <sys/utsname.h>

const char *
kernel_release([[maybe_unused]] const char *unused)
{
	struct utsname udata;

	if (uname(&udata) < 0) {
		warn("uname");
		return NULL;
	}

	return bprintf("%s", udata.release);
}
