/* See LICENSE file for copyright and license details. */
#include "../util.h"

#include <stdio.h>
#include <sys/statvfs.h>

const char *
disk_free(const char *path)
{
	struct statvfs fs;

	if (statvfs(path, &fs) < 0) {
		warn("statvfs '%s':", path);
		return NULL;
	}

	return fmt_human(fs.f_frsize * fs.f_bavail, 1024);
}

const char *
disk_perc(const char *path)
{
	struct statvfs fs;

	if (statvfs(path, &fs) < 0) {
		warn("statvfs '%s':", path);
		return NULL;
	}

	return bprintf("%.0f", 100 *
	               (1 - (double)fs.f_bavail / (double)fs.f_blocks));
}

const char *
disk_total(const char *path)
{
	struct statvfs fs;

	if (statvfs(path, &fs) < 0) {
		warn("statvfs '%s':", path);
		return NULL;
	}

	return fmt_human(fs.f_frsize * fs.f_blocks, 1024);
}

const char *
disk_used(const char *path)
{
	struct statvfs fs;

	if (statvfs(path, &fs) < 0) {
		warn("statvfs '%s':", path);
		return NULL;
	}

	return fmt_human(fs.f_frsize * (fs.f_blocks - fs.f_bfree), 1024);
}
