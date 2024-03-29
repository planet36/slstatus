/* See LICENSE file for copyright and license details. */
#include "../slstatus.h"
#include "../util.h"

#include <dirent.h>
#include <err.h>
#include <stdio.h>
#include <string.h>

const char *
num_files(const char *path)
{
	struct dirent *dp;
	DIR *dir;
	int num;

	if (!(dir = opendir(path))) {
		warn("opendir '%s'", path);
		return NULL;
	}

	num = 0;
	while ((dp = readdir(dir))) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue; /* skip self and parent */

		num++;
	}

	if (closedir(dir) < 0)
		warn("closedir '%s'", path);

	return bprintf("%d", num);
}
