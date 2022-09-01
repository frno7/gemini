// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2021 Fredrik Noring
 */

#include <stdarg.h>

#include <gem/fnt.h>

#include "internal/build-assert.h"
#include "internal/print.h"

int32_t fnt_char_offset(uint16_t c, const struct fnt *fnt)
{
	if (c < fnt->header->first || c > fnt->header->last + 1)
		return -1;

	const uint16_t k = c - fnt->header->first;
	const uint8_t *b = (const uint8_t *)fnt->header;

	return b[fnt->header->character_offsets + 2 * k + 0] |
	      (b[fnt->header->character_offsets + 2 * k + 1] << 8);
}

int16_t fnt_char_horizontal(uint16_t c, const struct fnt *fnt)
{
	if (c < fnt->header->first || c > fnt->header->last)
		return 0;

	const uint16_t k = c - fnt->header->first;
	const uint8_t *b = (const uint8_t *)fnt->header;

	return b[fnt->header->horizontal_offsets + 2 * k + 0] |
	      (b[fnt->header->horizontal_offsets + 2 * k + 1] << 8);
}

int fnt_char_width(const uint16_t c, const struct fnt *fnt)
{
	if (c < fnt->header->first || c > fnt->header->last)
		return -1;

	const int32_t x0 = fnt_char_offset(c + 0, fnt);
	const int32_t x1 = fnt_char_offset(c + 1, fnt);

	return x0 >= 0 && x1 >= 0 && x0 <= x1 ? x1 - x0 : -1;
}

bool fnt_char_pixel(const int x, const int y,
	const uint16_t c, const struct fnt *fnt)
{
	const int w = fnt_char_width(c, fnt);

	if (w <= 0)
		return false;

	if (c < fnt->header->first || c > fnt->header->last ||
	    x < 0 || x >= w ||
	    y < 0 || y >= fnt->header->bitmap_lines)
		return false;

	const int32_t x0 = fnt_char_offset(c, fnt);

	if (x0 < 0)
		return false;

	/* FIXME: Consider fnt->header->flags.big_endian */

	const uint8_t *b = (const uint8_t *)fnt->header;
	const uint8_t *d = &b[fnt->header->character_bitmap +
			      fnt->header->bitmap_stride * y + ((x0 + x) / 8)];

	return (*d & (0x80 >> ((x0 + x) % 8))) != 0;
}

static void report(void (*f)(const char *msg, void *arg), void *arg,
	const char *prefix, const char *suffix, const char *fmt, va_list ap)
{
	if (!f)
		return;

	char buf[2048];
	char msg[4096];

	vsnprintf(buf, sizeof(buf), fmt, ap);
	snprintf(msg, sizeof(msg), "%s%s%s%s%s",
		prefix, prefix[0] ? ": " : "",
		suffix, suffix[0] ? ": " : "", buf);

	f(msg, arg);
}

static bool __attribute__((format(printf, 3, 4))) fnt_error(void *arg,
	const struct fnt_diagnostic *diagnostic, char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	report(diagnostic->error, arg, "", "", fmt, ap);
	va_end(ap);

	return false;
}

bool fnt_valid_diagnostic(const struct fnt *fnt,
	const struct fnt_diagnostic *diagnostic, void *arg)
{
	BUILD_BUG_ON(sizeof(struct fnt_header_name) != 32);
	BUILD_BUG_ON(sizeof(struct fnt_header_flags) != 2);
	BUILD_BUG_ON(sizeof(struct fnt_header) != 88);

	if (fnt->size < sizeof(*fnt->header))
		return fnt_error(arg, diagnostic,
			"Header %zu bytes too small", fnt->size);

	if (fnt->header->first > fnt->header->last)
		return fnt_error(arg, diagnostic,
			"First character %u greater than last %u",
			fnt->header->first, fnt->header->last);

	const size_t bitmap_size = fnt->header->bitmap_stride *
				   fnt->header->bitmap_lines;
	const size_t bitmap_width = 8 * fnt->header->bitmap_stride;
	const uint16_t character_count =
		1 + fnt->header->last - fnt->header->first;

	if (fnt->header->flags.horizontal &&
	    fnt->header->horizontal_offsets + 2 * character_count > fnt->size)
		return fnt_error(arg, diagnostic,
			"Horizontal offset table beyond end of file");

	if (fnt->header->character_offsets +
			2 * (character_count + 1) > fnt->size)
		return fnt_error(arg, diagnostic,
			"Character offset table beyond end of file");

	for (int c = fnt->header->first; c <= fnt->header->last; c++)
		if (fnt_char_offset(c + 0, fnt) < 0)
			return fnt_error(arg, diagnostic,
				"Malformed offset character 0x%x", c);
		else if (fnt_char_offset(c + 0, fnt) >
		         fnt_char_offset(c + 1, fnt))
			return fnt_error(arg, diagnostic,
				"Unordered offset characters 0x%x to 0x%x offsets %d to %d",
				c, c + 1,
				fnt_char_offset(c + 0, fnt),
				fnt_char_offset(c + 1, fnt));
		else if (fnt_char_offset(c + 1, fnt) > bitmap_width)
			return fnt_error(arg, diagnostic,
				"Offset character 0x%x beyond bitmap width",
				c + 1);

	if (fnt->header->bitmap_stride % 2 != 0)
		return fnt_error(arg, diagnostic,
			"Uneven bitmap stride %u", fnt->header->bitmap_stride);

	if (fnt->header->character_bitmap + bitmap_size > fnt->size)
		return fnt_error(arg, diagnostic,
			"Character bitmap beyond end of file");

	return true;
}

bool fnt_valid(const struct fnt *fnt)
{
	const struct fnt_diagnostic diagnostic = { };

	return fnt_valid_diagnostic(fnt, &diagnostic, NULL);
}
