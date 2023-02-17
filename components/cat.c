/* See LICENSE file for copyright and license details. */
#include "../slstatus.h"
#include "../util.h"

#include <err.h>
#include <stdio.h>
#include <string.h>

const char *
cat(const char *path)
{
	char *f;
	FILE *fp;

	fp = fopen(path, "r");
	if (fp == NULL) {
		warn("fopen '%s':", path);
		return NULL;
	}

	f = fgets(buf, sizeof(buf) - 1, fp);
	if (fclose(fp) < 0) {
		warn("fclose '%s':", path);
		return NULL;
	}
	if (!f)
		return NULL;

	f = strrchr(buf, '\n');
	if (f != NULL)
		f[0] = '\0';

	return buf[0] ? buf : NULL;
}
