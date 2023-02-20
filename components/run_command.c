/* See LICENSE file for copyright and license details. */
#include "../slstatus.h"
#include "../util.h"

#include <err.h>
#include <stdio.h>
#include <string.h>

const char *
run_command(const char *cmd)
{
	char *p;
	FILE *fp;

	fp = popen(cmd, "r");
	if (fp == NULL) {
		warn("popen '%s'", cmd);
		return NULL;
	}

	p = fgets(buf, sizeof(buf) - 1, fp);
	if (pclose(fp) < 0) {
		warn("pclose '%s'", cmd);
		return NULL;
	}
	if (p == NULL)
		return NULL;

	p = strrchr(buf, '\n');
	if (p != NULL)
		p[0] = '\0';

	return buf[0] ? buf : NULL;
}
