// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2021 Fredrik Noring
 */

#ifndef _GEM_FNT_H
#define _GEM_FNT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "internal/struct.h"

struct fnt {
	size_t size;
	const void *data;
};

struct fnt_header_name {
	char s[32];
} LE_STORAGE PACKED;

#define FNT_HEADER_FLAGS_FIELD(f)					\
	f(0, system_font)						\
	f(1, horizontal)						\
	f(2, big_endian)						\
	f(3, monospace)

struct fnt_header_flags {
#define FNT_HEADER_FLAGS_STRUCTURE(n_, symbol_)				\
	uint16_t symbol_ : 1;
FNT_HEADER_FLAGS_FIELD(FNT_HEADER_FLAGS_STRUCTURE)
	uint16_t undefined : 12;
} LE_STORAGE PACKED;

#define FNT_HEADER_FIELD(f)						\
	f(uint16_t,                id,                 integer)		\
	f(uint16_t,                point,              integer)		\
	f(struct fnt_header_name,  name,               name)		\
	f(uint16_t,                first,              integer)		\
	f(uint16_t,                last,               integer)		\
	f(uint16_t,                top,                integer)		\
	f(uint16_t,                ascent,             integer)		\
	f(uint16_t,                half,               integer)		\
	f(uint16_t,                descent,            integer)		\
	f(uint16_t,                bottom,             integer)		\
	f(uint16_t,                max_char_width,     integer)		\
	f(uint16_t,                max_cell_width,     integer)		\
	f(uint16_t,                slant_left_offset,  integer)		\
	f(uint16_t,                slant_right_offset, integer)		\
	f(uint16_t,                thicken_size,       integer)		\
	f(uint16_t,                underline_size,     integer)		\
	f(uint16_t,                lighten_mask,       mask)		\
	f(uint16_t,                skew_mask,          mask)		\
	f(struct fnt_header_flags, flags,              flags)		\
	f(uint32_t,                horizontal_offsets, integer)		\
	f(uint32_t,                character_offsets,  integer)		\
	f(uint32_t,                character_bitmap,   integer)		\
	f(uint16_t,                bitmap_stride,      integer)		\
	f(uint16_t,                bitmap_lines,       integer)		\
	f(uint32_t,                next,               integer)

struct fnt_header {
#define FNT_HEADER_STRUCTURE(type_, symbol_, form_)			\
	type_ symbol_;
FNT_HEADER_FIELD(FNT_HEADER_STRUCTURE)
} LE_STORAGE PACKED;

int32_t fnt_char_offset(uint16_t c, const struct fnt *fnt);

int16_t fnt_char_horizontal(uint16_t c, const struct fnt *fnt);

int fnt_char_width(const uint16_t c, const struct fnt *fnt);

bool fnt_valid(const struct fnt *fnt);

struct fnt_diagnostic {
	void (*warning)(const char *msg, void *arg);
	void (*error)(const char *msg, void *arg);
};

bool fnt_valid_diagnostic(const struct fnt *fnt,
	const struct fnt_diagnostic *diagnostic, void *arg);

#endif /* _GEM_FNT_H */
