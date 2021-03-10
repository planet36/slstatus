/* See LICENSE file for copyright and license details. */
#include "../util.h"

const char *
counter(void)
{
	static uintmax_t i;
	return bprintf("%ju", i++);
}
