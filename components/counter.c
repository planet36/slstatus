// SPDX-FileCopyrightText: Steven Ward
// SPDX-License-Identifier: OSL-3.0

#include "../slstatus.h"
#include "../util.h"

const char *
counter([[maybe_unused]] const char *unused)
{
	static uintmax_t i;
	return bprintf("%ju", i++);
}
