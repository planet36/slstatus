// SPDX-FileCopyrightText: Steven Ward
// SPDX-License-Identifier: MPL-2.0

#include "../slstatus.h"
#include "../util.h"

const char *
counter([[maybe_unused]] const char *unused)
{
	static uintmax_t i;
	return bprintf("%ju", i++);
}
