// SPDX-FileCopyrightText: Steven Ward
// SPDX-License-Identifier: OSL-3.0

#include "meter.h"
#include "util.h"

// https://www.unicode.org/charts/PDF/U0000.pdf

static const wchar_t SPACE = 0x0020;

// https://www.unicode.org/charts/PDF/U2580.pdf

static const wchar_t UPPER_HALF_BLOCK           = 0x2580;
static const wchar_t LOWER_ONE_EIGHTH_BLOCK     = 0x2581;
static const wchar_t LOWER_ONE_QUARTER_BLOCK    = 0x2582;
static const wchar_t LOWER_THREE_EIGHTHS_BLOCK  = 0x2583;
static const wchar_t LOWER_HALF_BLOCK           = 0x2584;
static const wchar_t LOWER_FIVE_EIGHTHS_BLOCK   = 0x2585;
static const wchar_t LOWER_THREE_QUARTERS_BLOCK = 0x2586;
static const wchar_t LOWER_SEVEN_EIGHTHS_BLOCK  = 0x2587;
static const wchar_t FULL_BLOCK                 = 0x2588;
static const wchar_t LEFT_SEVEN_EIGHTHS_BLOCK   = 0x2589;
static const wchar_t LEFT_THREE_QUARTERS_BLOCK  = 0x258A;
static const wchar_t LEFT_FIVE_EIGHTHS_BLOCK    = 0x258B;
static const wchar_t LEFT_HALF_BLOCK            = 0x258C;
static const wchar_t LEFT_THREE_EIGHTHS_BLOCK   = 0x258D;
static const wchar_t LEFT_ONE_QUARTER_BLOCK     = 0x258E;
static const wchar_t LEFT_ONE_EIGHTH_BLOCK      = 0x258F;
static const wchar_t RIGHT_HALF_BLOCK           = 0x2590;

static const wchar_t UPPER_ONE_EIGHTH_BLOCK     = 0x2594;
static const wchar_t RIGHT_ONE_EIGHTH_BLOCK     = 0x2595;

// https://www.unicode.org/charts/PDF/U1FB00.pdf

static const wchar_t VERTICAL_ONE_EIGHTH_BLOCK_2   = 0x1FB70;
static const wchar_t VERTICAL_ONE_EIGHTH_BLOCK_3   = 0x1FB71;
static const wchar_t VERTICAL_ONE_EIGHTH_BLOCK_4   = 0x1FB72;
static const wchar_t VERTICAL_ONE_EIGHTH_BLOCK_5   = 0x1FB73;
static const wchar_t VERTICAL_ONE_EIGHTH_BLOCK_6   = 0x1FB74;
static const wchar_t VERTICAL_ONE_EIGHTH_BLOCK_7   = 0x1FB75;
static const wchar_t HORIZONTAL_ONE_EIGHTH_BLOCK_2 = 0x1FB76;
static const wchar_t HORIZONTAL_ONE_EIGHTH_BLOCK_3 = 0x1FB77;
static const wchar_t HORIZONTAL_ONE_EIGHTH_BLOCK_4 = 0x1FB78;
static const wchar_t HORIZONTAL_ONE_EIGHTH_BLOCK_5 = 0x1FB79;
static const wchar_t HORIZONTAL_ONE_EIGHTH_BLOCK_6 = 0x1FB7A;
static const wchar_t HORIZONTAL_ONE_EIGHTH_BLOCK_7 = 0x1FB7B;

static const wchar_t UPPER_ONE_QUARTER_BLOCK    = 0x1FB82;
static const wchar_t UPPER_THREE_EIGHTHS_BLOCK  = 0x1FB83;
static const wchar_t UPPER_FIVE_EIGHTHS_BLOCK   = 0x1FB84;
static const wchar_t UPPER_THREE_QUARTERS_BLOCK = 0x1FB85;
static const wchar_t UPPER_SEVEN_EIGHTHS_BLOCK  = 0x1FB86;
static const wchar_t RIGHT_ONE_QUARTER_BLOCK    = 0x1FB87;
static const wchar_t RIGHT_THREE_EIGHTHS_BLOCK  = 0x1FB88;
static const wchar_t RIGHT_FIVE_EIGHTHS_BLOCK   = 0x1FB89;
static const wchar_t RIGHT_THREE_QUARTERS_BLOCK = 0x1FB8A;
static const wchar_t RIGHT_SEVEN_EIGHTHS_BLOCK  = 0x1FB8B;

static const wchar_t lower_blocks[] = {
	SPACE,
	LOWER_ONE_EIGHTH_BLOCK,
	LOWER_ONE_QUARTER_BLOCK,
	LOWER_THREE_EIGHTHS_BLOCK,
	LOWER_HALF_BLOCK,
	LOWER_FIVE_EIGHTHS_BLOCK,
	LOWER_THREE_QUARTERS_BLOCK,
	LOWER_SEVEN_EIGHTHS_BLOCK,
	FULL_BLOCK,
};

static const size_t num_lower_blocks = LEN(lower_blocks);

static const wchar_t left_blocks[] = {
	SPACE,
	LEFT_ONE_EIGHTH_BLOCK,
	LEFT_ONE_QUARTER_BLOCK,
	LEFT_THREE_EIGHTHS_BLOCK,
	LEFT_HALF_BLOCK,
	LEFT_FIVE_EIGHTHS_BLOCK,
	LEFT_THREE_QUARTERS_BLOCK,
	LEFT_SEVEN_EIGHTHS_BLOCK,
	FULL_BLOCK,
};

static const size_t num_left_blocks = LEN(left_blocks);

static const wchar_t ver_lines[] = {
	LEFT_ONE_EIGHTH_BLOCK,
	VERTICAL_ONE_EIGHTH_BLOCK_2,
	VERTICAL_ONE_EIGHTH_BLOCK_3,
	VERTICAL_ONE_EIGHTH_BLOCK_4,
	VERTICAL_ONE_EIGHTH_BLOCK_5,
	VERTICAL_ONE_EIGHTH_BLOCK_6,
	VERTICAL_ONE_EIGHTH_BLOCK_7,
	RIGHT_ONE_EIGHTH_BLOCK,
};

static const size_t num_ver_lines = LEN(ver_lines);

static const wchar_t hor_lines[] = {
	LOWER_ONE_EIGHTH_BLOCK,
	HORIZONTAL_ONE_EIGHTH_BLOCK_7,
	HORIZONTAL_ONE_EIGHTH_BLOCK_6,
	HORIZONTAL_ONE_EIGHTH_BLOCK_5,
	HORIZONTAL_ONE_EIGHTH_BLOCK_4,
	HORIZONTAL_ONE_EIGHTH_BLOCK_3,
	HORIZONTAL_ONE_EIGHTH_BLOCK_2,
	UPPER_ONE_EIGHTH_BLOCK,
};

static const size_t num_hor_lines = LEN(hor_lines);

static const wchar_t upper_blocks[] = {
	SPACE,
	UPPER_ONE_EIGHTH_BLOCK,
	UPPER_ONE_QUARTER_BLOCK,
	UPPER_THREE_EIGHTHS_BLOCK,
	UPPER_HALF_BLOCK,
	UPPER_FIVE_EIGHTHS_BLOCK,
	UPPER_THREE_QUARTERS_BLOCK,
	UPPER_SEVEN_EIGHTHS_BLOCK,
	FULL_BLOCK,
};

static const size_t num_upper_blocks = LEN(upper_blocks);

static const wchar_t right_blocks[] = {
	SPACE,
	RIGHT_ONE_EIGHTH_BLOCK,
	RIGHT_ONE_QUARTER_BLOCK,
	RIGHT_THREE_EIGHTHS_BLOCK,
	RIGHT_HALF_BLOCK,
	RIGHT_FIVE_EIGHTHS_BLOCK,
	RIGHT_THREE_QUARTERS_BLOCK,
	RIGHT_SEVEN_EIGHTHS_BLOCK,
	FULL_BLOCK,
};

static const size_t num_right_blocks = LEN(right_blocks);

static double
my_fmod(double x, double y)
{
	return x - (intmax_t)(x / y) * y;
}

static void
clamp(double* x)
{
	if (*x < 0)
		*x = 0;
	else if (*x > 1)
		*x = 1;
}

/* Map the real number in the interval [0, 1]
 * to an integer in the interval [0, b).
 */
static size_t
map_to_uint(double x, size_t b)
{
	clamp(&x);

	return (size_t)(x * (b - 1U) + 0.5); // round to nearest int
}

/* x is in the interval [0, 1].
 *
 * meter_width does not include the terminating null character.
 *
 * blocks_len is the length of the wide character array used for the meter.
 */
static void
calc_meter_segments(double x, size_t meter_width, size_t blocks_len,
                    size_t* left_width, size_t* blocks_index, size_t* right_width)
{
	*left_width = 0;
	*blocks_index = -1;
	*right_width = 0;

	if (meter_width == 0)
		return;

	clamp(&x);

	*left_width = (size_t)(x * meter_width);

	if (*left_width == meter_width)
		return;

	/* Using "blocks_len - 1" instead of "blocks_len" in the calculations
	 * produces a preferred distribution at the ends of the meter.
	 */
	// *blocks_index = (size_t)my_fmod(x * meter_width * blocks_len, blocks_len);
	*blocks_index = (size_t)(my_fmod(x * meter_width * (blocks_len - 1U),
	                              blocks_len - 1U) + 0.5);

	*right_width = meter_width - *left_width - 1U;
}

/* Return a Unicode lower block character (whose height is proportional to x).
 *
 * x is in the interval [0, 1].
 */
wchar_t
lower_blocks_1(double x)
{
	return lower_blocks[map_to_uint(x, num_lower_blocks)];
}

/* Return a Unicode horizontal 1/8 block character (whose height is proportional to x).
 */
wchar_t
hor_lines_1(double x)
{
	return hor_lines[map_to_uint(x, num_hor_lines)];
}

/* Return a Unicode upper block character (whose height is proportional to x).
 */
wchar_t
upper_blocks_1(double x)
{
	return upper_blocks[map_to_uint(x, num_upper_blocks)];
}

/* Fill a meter with Unicode left block characters.
 * The filled region is proportional to x, starting at the left.
 *
 * x is in the interval [0, 1].
 *
 * meter is a buffer capable of holding meter_width wide characters
 * (not including the terminating null character).
 *
 * It is the caller's responsibility to null-terminate the meter buffer.
 */
void
left_blocks_meter(double x, wchar_t* meter, size_t meter_width)
{
	size_t left_width, blocks_index, right_width, i;

	if (meter_width == 0)
		return;

	calc_meter_segments(x, meter_width, num_left_blocks,
	                    &left_width, &blocks_index, &right_width);

	for (i = 0; i < left_width; ++i)
	{
		*meter++ = FULL_BLOCK;
	}

	if (blocks_index != (size_t)-1)
		*meter++ = left_blocks[blocks_index];

	for (i = 0; i < right_width; ++i)
	{
		*meter++ = SPACE;
	}
}

/* Fill a meter with a Unicode vertical 1/8 block character.
 * The position of the filled character is proportional to x, starting at the left.
 */
void
ver_lines_meter(double x, wchar_t* meter, size_t meter_width)
{
	size_t left_width, blocks_index, right_width, i;

	if (meter_width == 0)
		return;

	calc_meter_segments(x, meter_width, num_ver_lines,
	                    &left_width, &blocks_index, &right_width);

	if (left_width == meter_width)
	{
		// Special case when the vertical line will be at the right end

		for (i = 1; i < left_width; ++i)
		{
			*meter++ = SPACE;
		}

		*meter++ = RIGHT_ONE_EIGHTH_BLOCK;
	}
	else
	{
		for (i = 0; i < left_width; ++i)
		{
			*meter++ = SPACE;
		}

		if (blocks_index != (size_t)-1)
			*meter++ = ver_lines[blocks_index];

		for (i = 0; i < right_width; ++i)
		{
			*meter++ = SPACE;
		}
	}
}

/* Fill a meter with Unicode right block characters.
 * The filled region is proportional to x, starting at the right.
 */
void
right_blocks_meter(double x, wchar_t* meter, size_t meter_width)
{
	size_t left_width, blocks_index, right_width, i;

	if (meter_width == 0)
		return;

	calc_meter_segments(x, meter_width, num_right_blocks,
	                    &left_width, &blocks_index, &right_width);

	// left_width and right_width are swapped

	for (i = 0; i < right_width; ++i)
	{
		*meter++ = SPACE;
	}

	if (blocks_index != (size_t)-1)
		*meter++ = right_blocks[blocks_index];

	for (i = 0; i < left_width; ++i)
	{
		*meter++ = FULL_BLOCK;
	}
}

/* Fill a meter with ASCII characters.
 * The filled region is proportional to x, starting at the left.
 *
 * x is in the interval [0, 1].
 *
 * meter is a buffer capable of holding meter_width characters
 * (not including the terminating null character).
 *
 * It is the caller's responsibility to null-terminate the meter buffer.
 */
void
left_char_meter(double x, char* meter, size_t meter_width, char fill, char unfill)
{
	size_t i;

	if (meter_width == 0)
		return;

	clamp(&x);

	// round to nearest int
	const size_t num_filled = (size_t)(x * meter_width + 0.5);

	for (i = 0; i < num_filled; ++i)
	{
		meter[i] = fill;
	}
	for (; i < meter_width; ++i)
	{
		meter[i] = unfill;
	}
}

/* Fill a meter with ASCII characters.
 * The filled region is proportional to x, starting at the right.
 */
void
right_char_meter(double x, char* meter, size_t meter_width, char fill, char unfill)
{
	size_t i;

	if (meter_width == 0)
		return;

	clamp(&x);

	// round to nearest int
	const size_t num_unfilled = meter_width - (size_t)(x * meter_width + 0.5);

	for (i = 0; i < num_unfilled; ++i)
	{
		meter[i] = unfill;
	}
	for (; i < meter_width; ++i)
	{
		meter[i] = fill;
	}
}
