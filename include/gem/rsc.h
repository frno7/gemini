// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#ifndef _GEM_RSC_H
#define _GEM_RSC_H

#include <stdbool.h>
#include <stdint.h>

#include "color.h"
#include "object.h"

#include "internal/struct.h"

struct rsc {
	size_t size;
	const struct rsc_header *header;
};

#define RSC_HEADER_FIELD(f)						\
	f(uint16_t, vrsn)						\
	f(uint16_t, object)						\
	f(uint16_t, tedinfo)						\
	f(uint16_t, iconblk)						\
	f(uint16_t, bitblk)						\
	f(uint16_t, frstr)						\
	f(uint16_t, string)						\
	f(uint16_t, imdata)						\
	f(uint16_t, frimg)						\
	f(uint16_t, trindex)						\
	f(uint16_t, nobs)						\
	f(uint16_t, ntree)						\
	f(uint16_t, nted)						\
	f(uint16_t, nib)						\
	f(uint16_t, nbb)						\
	f(uint16_t, nstring)						\
	f(uint16_t, nimages)						\
	f(uint16_t, rssize)

struct rsc_header {
#define RSC_HEADER_STRUCTURE(type_, name_) type_ rsh_ ## name_;
RSC_HEADER_FIELD(RSC_HEADER_STRUCTURE)
} BE_STORAGE PACKED;

struct rsc_object_color {
	uint16_t border : 4;
	uint16_t text : 4;
	uint16_t opaque : 1;
	uint16_t pattern : 3;
	uint16_t fill : 4;
} BE_STORAGE PACKED;

struct rsc_object_spec {
	union {
		struct rsc_object_spec_box {
			char c;
			int8_t thickness;
			struct rsc_object_color color;
		} BE_STORAGE PACKED box;
		uint32_t tedinfo;
		uint32_t bitblk;
		uint32_t string;
		uint32_t iconblk;
		uint32_t applblk;
		uint32_t ciconblk;
		uint32_t undefined;
	} BE_STORAGE PACKED;
} BE_STORAGE PACKED;

struct rsc_object_type {
	uint8_t m;
	uint8_t g;
} BE_STORAGE PACKED;

struct rsc_object_flags {
	union {
		struct {
			uint16_t undefined : 4;
#define RSC_OBJECT_FLAG_STRUCT(bit_, symbol_, label_)			\
			uint16_t symbol_ : 1;
GEM_OBJECT_FLAG(RSC_OBJECT_FLAG_STRUCT)
		} BE_STORAGE PACKED;
		uint16_t mask;
	} BE_STORAGE PACKED;
} BE_STORAGE PACKED;

struct rsc_object_state {
	union {
		struct {
			uint16_t undefined : 10;
#define RSC_OBJECT_STATE_STRUCT(bit_, symbol_, label_)			\
			uint16_t symbol_ : 1;
GEM_OBJECT_STATE(RSC_OBJECT_STATE_STRUCT)
		} BE_STORAGE PACKED;
		uint16_t mask;
	} BE_STORAGE PACKED;
} BE_STORAGE PACKED;

struct rsc_object_grid {
	uint8_t px;
	uint8_t ch;
} BE_STORAGE PACKED;

struct rsc_object_point {
	struct rsc_object_grid x;
	struct rsc_object_grid y;
} BE_STORAGE PACKED;

struct rsc_object_rectangle {
	struct rsc_object_grid w;
	struct rsc_object_grid h;
} BE_STORAGE PACKED;

struct rsc_object_area {
	struct rsc_object_point p;
	struct rsc_object_rectangle r;
} BE_STORAGE PACKED;

struct rsc_object {
	struct rsc_object_link {
		int16_t next;
		int16_t head;
		int16_t tail;
	} BE_STORAGE PACKED link;
	struct rsc_object_attr {
		struct rsc_object_type type;
		struct rsc_object_flags flags;
		struct rsc_object_state state;
		struct rsc_object_spec spec;
		struct rsc_object_area area;
	} BE_STORAGE PACKED attr;
} BE_STORAGE PACKED;

enum {
#define RSC_TEDINFO_JUSTIFICATION_ENUM(n_, symbol_, label_)		\
	rsc_tedinfo_ ## symbol_ = n_,
GEM_TEDINFO_JUSTIFICATION(RSC_TEDINFO_JUSTIFICATION_ENUM)
};

#define RSC_TEDINFO_FIELD(f)						\
	f(uint32_t,                text,      string)			\
	f(uint32_t,                tmplt,     string)			\
	f(uint32_t,                valid,     string)			\
	f(int16_t,                 font,      font)			\
	f(int16_t,                 fontid,    integer)			\
	f(int16_t,                 just,      just)			\
	f(struct rsc_object_color, color,     color)			\
	f(int16_t,                 fontsize,  integer)			\
	f(int16_t,                 thickness, integer)			\
	f(int16_t,                 txtlen,    integer)			\
	f(int16_t,                 tmplen,    integer)

struct rsc_tedinfo {
#define RSC_TEDINFO_STRUCTURE(type_, name_, form_)			\
	type_ te_ ## name_;
RSC_TEDINFO_FIELD(RSC_TEDINFO_STRUCTURE)
} BE_STORAGE PACKED;

#define RSC_BITBLK_FIELD(f)						\
	f(uint32_t, data)						\
	f(int16_t,  wb)							\
	f(int16_t,  hl)							\
	f(int16_t,  x)							\
	f(int16_t,  y)							\
	f(int16_t,  color)

struct rsc_bitblk {
#define RSC_BITBLK_STRUCTURE(type_, name_) type_ bi_ ## name_;
RSC_BITBLK_FIELD(RSC_BITBLK_STRUCTURE)
} BE_STORAGE PACKED;

struct rsc_iconblk_color {
	uint8_t fg : 4;
	uint8_t bg : 4;
} BE_STORAGE PACKED;

struct rsc_iconblk_point {
	int16_t x;
	int16_t y;
} BE_STORAGE PACKED;

struct rsc_iconblk_rectangle {
	int16_t w;
	int16_t h;
} BE_STORAGE PACKED;

struct rsc_iconblk_area {
	struct {
		struct rsc_iconblk_point p;
		struct rsc_iconblk_rectangle r;
	} BE_STORAGE PACKED;
} BE_STORAGE PACKED;

struct rsc_iconblk_char {
        struct rsc_iconblk_color color;
        char c;
        struct rsc_iconblk_point p;
} BE_STORAGE PACKED;

#define RSC_ICONBLK_FIELD(f)						\
        f(uint32_t,                mask)				\
        f(uint32_t,                data)				\
        f(uint32_t,                text)				\
        f(struct rsc_iconblk_char, char)				\
        f(struct rsc_iconblk_area, icon)				\
        f(struct rsc_iconblk_area, txt)

struct rsc_iconblk {
#define RSC_ICONBLK_STRUCTURE(type_, name_) type_ ib_ ## name_;
RSC_ICONBLK_FIELD(RSC_ICONBLK_STRUCTURE)
} BE_STORAGE PACKED;

struct rsc_applblk {
	uint32_t ap_code;
	uint32_t ap_parm;
} BE_STORAGE PACKED;

size_t rsc_string_offset_at_index(const size_t i, const struct rsc *rsc);

const char *rsc_string_at_offset(const size_t offset, const struct rsc *rsc);

const char *rsc_string_at_index(const size_t index, const struct rsc *rsc);

const struct rsc_tedinfo *rsc_tedinfo_at_offset(const size_t offset,
	const struct rsc *rsc);

const struct rsc_bitblk *rsc_bitblk_at_offset(const size_t offset,
	const struct rsc *rsc);

size_t rsc_frimg_offset_at_index(const size_t i, const struct rsc *rsc);

const struct rsc_bitblk *rsc_frimg_at_index(const size_t index,
	const struct rsc *rsc);

struct rsc_iconblk_pixel {
	uint8_t data : 1;
	uint8_t mask : 1;
};

struct rsc_iconblk_pixel rsc_iconblk_pixel(int x, int y,
	const struct rsc_iconblk *iconblk, const struct rsc *rsc);

const uint8_t *rsc_bitmap_at_offset(const size_t offset, const struct rsc *rsc);

bool rsc_bitblk_pixel(int x, int y,
	const struct rsc_bitblk *bitblk, const struct rsc *rsc);

const struct rsc_iconblk *rsc_iconblk_at_offset(const size_t offset,
	const struct rsc *rsc);

size_t rsc_iconblk_offset_at_index(const size_t i, const struct rsc *rsc);

const struct rsc_iconblk *rsc_iconblk_at_index(const size_t i,
	const struct rsc *rsc);

const struct rsc_object *rsc_tree_object_at_offset(
	const size_t offset, const struct rsc *rsc);

const struct rsc_object *rsc_tree_at_index(
	const size_t index, const struct rsc *rsc);

size_t rsc_unextended_size(const struct rsc *rsc);

int16_t rsc_object_parent(int16_t ob, const struct rsc_object *tree);

int16_t rsc_tree_traverse(int16_t ob, const struct rsc_object *tree);

static inline bool rsc_valid_ob(const int16_t ob) { return ob >= 0; }

#define RSC_MAP_ENTRY_TYPE(t)						\
	t( 0, unused)							\
	t( 1, header)							\
	t( 2, object)							\
	t( 3, tedinfo)							\
	t( 4, iconblk)							\
	t( 5, bitblk)							\
	t( 6, applblk)							\
	t( 7, frstr)							\
	t( 8, string)							\
	t( 9, imdata)							\
	t(10, frimg)							\
	t(11, trindex)							\
	t(12, padding)

enum rsc_map_entry_type {
#define RSC_MAP_ENTRY_TYPE_ENUM(n_, symbol_)				\
	rsc_map_entry_type_ ## symbol_ = n_,
RSC_MAP_ENTRY_TYPE(RSC_MAP_ENTRY_TYPE_ENUM)
};

struct rsc_map_entry {
	uint8_t start : 1;
	uint8_t reserved : 1;
	uint8_t type : 6;
};

struct rsc_map {
	size_t size;
	struct rsc_map_entry *entry;
};

struct rsc_map_region {
	struct rsc_map_entry entry;
	size_t offset;
	size_t size;
};

struct rsc_diagnostic {
	void (*warning)(const char *msg, void *arg);
	void (*error)(const char *msg, void *arg);
};

#define rsc_map_for_each_region(region_, map_)				\
	for ((region_) = rsc_map_first_region(map_);			\
	     (region_).size;						\
	     (region_) = rsc_map_next_region((region_), map_))

struct rsc_map_region rsc_map_first_region(struct rsc_map *map);

struct rsc_map_region rsc_map_next_region(
	struct rsc_map_region region, struct rsc_map *map);

struct rsc_map *rsc_map_alloc(const struct rsc *rsc);

void rsc_map_free(struct rsc_map *map);

bool rsc_map(struct rsc_map *map, const struct rsc *rsc);

const char *rsc_map_entry_type_symbol(enum rsc_map_entry_type type);

bool rsc_valid_structure_diagnostic(const struct rsc *rsc,
	const struct rsc_diagnostic *diagnostic, void *arg);

bool rsc_valid_structure(const struct rsc *rsc);

#endif /* _GEM_RSC_H */
