/* See LICENSE file for copyright and license details. */
#include "../meter.h"
#include "../util.h"

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <sys/statvfs.h>

/* percentages will be clamped to 99 */
#define MAX_PCT_99

#define METER_WIDTH 10
static_assert(METER_WIDTH > 0, "METER_WIDTH must be > 0");

static struct statvfs fs;

static int
update_fs(const char *path)
{
	if (statvfs(path, &fs) < 0) {
		warn("statvfs '%s'", path);
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
disk_meter(const char *path)
{
	double used;
	wchar_t meter[METER_WIDTH + 1] = {'\0'};

	if (update_fs(path) < 0) {
		return NULL;
	}

	used = 1 - (double)fs.f_bavail / fs.f_blocks;

	left_blocks_meter(used, meter, METER_WIDTH);

	return bprintf("%ls", meter);
}

const char *
disk_perc(const char *path)
{
	double used;

	if (update_fs(path) < 0) {
		return NULL;
	}

	used = 1 - (double)fs.f_bavail / fs.f_blocks;

#ifdef MAX_PCT_99
	if (used > 0.99)
		used = 0.99;
#endif

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
