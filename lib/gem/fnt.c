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
	const struct fnt_header *h = fnt->data;

	if (c < h->first || c > h->last + 1)
		return -1;

	const uint16_t k = c - h->first;
	const uint8_t *b = (const uint8_t *)fnt->data;

	return b[h->character_offsets + 2 * k + 0] |
	      (b[h->character_offsets + 2 * k + 1] << 8);
}

int16_t fnt_char_horizontal(uint16_t c, const struct fnt *fnt)
{
	const struct fnt_header *h = fnt->data;

	if (c < h->first || c > h->last)
		return 0;

	const uint16_t k = c - h->first;
	const uint8_t *b = (const uint8_t *)fnt->data;

	return b[h->horizontal_offsets + 2 * k + 0] |
	      (b[h->horizontal_offsets + 2 * k + 1] << 8);
}

int fnt_char_width(const uint16_t c, const struct fnt *fnt)
{
	const struct fnt_header *h = fnt->data;

	if (c < h->first || c > h->last)
		return -1;

	const int32_t x0 = fnt_char_offset(c + 0, fnt);
	const int32_t x1 = fnt_char_offset(c + 1, fnt);

	return x0 >= 0 && x1 >= 0 && x0 <= x1 ? x1 - x0 : -1;
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

	const struct fnt_header *h = fnt->data;

	if (fnt->size < sizeof(*h))
		return fnt_error(arg, diagnostic,
			"Header %zu bytes too small", fnt->size);

	if (h->first > h->last)
		return fnt_error(arg, diagnostic,
			"First character %u greater than last %u",
			h->first, h->last);

	const size_t bitmap_size = h->bitmap_stride * h->bitmap_lines;
	const size_t bitmap_width = 8 * h->bitmap_stride;
	const uint16_t character_count = 1 + h->last - h->first;

	if (h->flags.horizontal &&
	    h->horizontal_offsets + 2 * character_count > fnt->size)
		return fnt_error(arg, diagnostic,
			"Horizontal offset table beyond end of file");

	if (h->character_offsets + 2 * (character_count + 1) > fnt->size)
		return fnt_error(arg, diagnostic,
			"Character offset table beyond end of file");

	for (int c = h->first; c <= h->last; c++)
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

	if (h->bitmap_stride % 2 != 0)
		return fnt_error(arg, diagnostic,
			"Uneven bitmap stride %u", h->bitmap_stride);

	if (h->character_bitmap + bitmap_size > fnt->size)
		return fnt_error(arg, diagnostic,
			"Character bitmap beyond end of file");

	return true;
}

bool fnt_valid(const struct fnt *fnt)
{
	const struct fnt_diagnostic diagnostic = { };

	return fnt_valid_diagnostic(fnt, &diagnostic, NULL);
}
