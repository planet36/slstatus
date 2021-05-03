// SPDX-FileCopyrightText: Steven Ward
// SPDX-License-Identifier: OSL-3.0

#pragma once

#include <wchar.h>

wchar_t lower_blocks_1(double x);
wchar_t hor_lines_1(double x);
wchar_t upper_blocks_1(double x);
void left_blocks_meter(double x, wchar_t *meter, size_t meter_width);
void ver_lines_meter(double x, wchar_t *meter, size_t meter_width);
void right_blocks_meter(double x, wchar_t *meter, size_t meter_width);
void left_cmeter(double x, char* meter, size_t meter_width, char fill, char unfill);
void right_cmeter(double x, char* meter, size_t meter_width, char fill, char unfill);
