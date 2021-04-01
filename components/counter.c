// SPDX-FileCopyrightText: Steven Ward
// SPDX-License-Identifier: OSL-3.0

#include "../util.h"

const char *
counter(void)
{
	static uintmax_t i;
	return bprintf("%ju", i++);
}
