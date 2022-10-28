/* See LICENSE file for copyright and license details. */
#include "../slstatus.h"
#include "../util.h"

#include <err.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

const char *
gid([[maybe_unused]] const char *unused)
{
	return bprintf("%d", getgid());
}

const char *
username([[maybe_unused]] const char *unused)
{
	struct passwd *pw;

	if (!(pw = getpwuid(geteuid()))) {
		warn("getpwuid '%d'", geteuid());
		return NULL;
	}

	return bprintf("%s", pw->pw_name);
}

const char *
uid([[maybe_unused]] const char *unused)
{
	return bprintf("%d", geteuid());
}
