/* See LICENSE file for copyright and license details. */
#include "../util.h"

#include <stdio.h>
#include <sys/statvfs.h>

static struct statvfs fs;

static int
update_fs(const char *path)
{
	if (statvfs(path, &fs) < 0) {
		warn("statvfs '%s':", path);
		return -1;
	}

	return 0;
}

const char *
disk_free(const char *path)
{
	if (update_fs(path) < 0) {
		return NULL;
	}

	return fmt_human(fs.f_frsize * fs.f_bavail, 1024);
}

const char *
disk_perc(const char *path)
{
	double used;

	if (update_fs(path) < 0) {
		return NULL;
	}

	used = 1 - (double)fs.f_bavail / fs.f_blocks;

	return bprintf("%.0f", 100 * used);
}

const char *
disk_total(const char *path)
{
	if (update_fs(path) < 0) {
		return NULL;
	}

	return fmt_human(fs.f_frsize * fs.f_blocks, 1024);
}

const char *
disk_used(const char *path)
{
	if (update_fs(path) < 0) {
		return NULL;
	}

	return fmt_human(fs.f_frsize * (fs.f_blocks - fs.f_bfree), 1024);
}
