/* See LICENSE file for copyright and license details. */
#include "../util.h"

#include <dirent.h>
#include <err.h>
#include <stdio.h>
#include <string.h>

const char *
num_files(const char *path)
{
	struct dirent *dp;
	DIR *fd;
	int num;

	if (!(fd = opendir(path))) {
		warn("opendir '%s'", path);
		return NULL;
	}

	num = 0;
	while ((dp = readdir(fd))) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
			continue; /* skip self and parent */
		}
		num++;
	}

	closedir(fd);

	return bprintf("%d", num);
}
