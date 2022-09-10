// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2021 Fredrik Noring
 */

#ifndef _GEM_AES_H
#define _GEM_AES_H

#include <stdbool.h>
#include <stdint.h>

#include "rsc.h"
#include "vdi.h"

struct aes_point {
	int x;
	int y;
};

struct aes_rectangle {
	int w;
	int h;
};

struct aes_area {
	struct aes_point p;
	struct aes_rectangle r;
};

struct aes {
	vdi_id_t vdi_id;
};

typedef struct {
	struct aes *aes_;
} aes_id_t;

struct aes_object_color {
	uint16_t border : 4;
	uint16_t text : 4;
	uint16_t opaque : 1;
	uint16_t pattern : 3;
	uint16_t fill : 4;
};

#define AES_TEDINFO_FIELD(f)						\
	f(char *,                  text,      string)			\
	f(char *,                  tmplt,     string)			\
	f(char *,                  valid,     string)			\
	f(int16_t,                 font,      font)			\
	f(int16_t,                 fontid,    integer)			\
	f(int16_t,                 just,      just)			\
	f(struct aes_object_color, color,     color)			\
	f(int16_t,                 fontsize,  integer)			\
	f(int16_t,                 thickness, integer)

struct aes_tedinfo {
#define AES_TEDINFO_STRUCTURE(type_, name_, form_)			\
	type_ name_;
AES_TEDINFO_FIELD(AES_TEDINFO_STRUCTURE)
};

enum {
#define AES_TEDINFO_JUSTIFICATION_ENUM(n_, symbol_, label_)		\
	aes_tedinfo_ ## symbol_ = n_,
GEM_TEDINFO_JUSTIFICATION(AES_TEDINFO_JUSTIFICATION_ENUM)
};

struct aes_object_type {
	uint8_t m;
	uint8_t g;
};

struct aes_object_flags {
#define AES_OBJECT_FLAG_STRUCT(bit_, symbol_, label_)			\
	uint16_t symbol_ : 1;
GEM_OBJECT_FLAG(AES_OBJECT_FLAG_STRUCT)
};

struct aes_object_state {
#define AES_OBJECT_STATE_STRUCT(bit_, symbol_, label_)			\
	uint16_t symbol_ : 1;
GEM_OBJECT_STATE(AES_OBJECT_STATE_STRUCT)
};

struct aes_bitblk {
	uint8_t *data;
	struct aes_area area;
	int16_t color;
};

struct aes_iconblk_char_color {
	uint8_t fg : 4;
	uint8_t bg : 4;
};

struct aes_iconblk {
	struct aes_iconblk_bitmap {
		uint8_t *mask;
		uint8_t *data;
		struct aes_area area;
	} bitmap;
	struct aes_iconblk_text {
		char *s;
		struct aes_area area;
	} text;
	struct aes_iconblk_char {
		char c;
		struct aes_iconblk_char_color color;
		struct aes_area area;
	} char_;
};

struct aes_iconblk_pixel {
	uint8_t data : 1;
	uint8_t mask : 1;
};

struct aes_object_spec {
	union {
		struct aes_object_spec_box {
			char c;
			int8_t thickness;
			struct aes_object_color color;
		} box;
		struct aes_tedinfo tedinfo;
		struct aes_bitblk bitblk;
		struct aes_iconblk iconblk;
		char *string;
	};
};

struct aes_object_shape {
	struct aes_object_type type;
	struct aes_object_flags flags;
	struct aes_object_state state;
	struct aes_object_spec spec;
	struct aes_area area;
};

aes_id_t aes_appl_init(struct aes *aes_);

static inline bool aes_id_valid(aes_id_t aes_id)
{
	return vdi_id_valid(aes_id.aes_->vdi_id);
}

struct fnt *aes_fnt_large(aes_id_t aes_id);

struct fnt *aes_fnt_small(aes_id_t aes_id);

void aes_appl_exit(aes_id_t aes_id);

struct aes_iconblk_pixel aes_iconblk_pixel(const struct aes_point p,
	const struct aes_iconblk *iconblk);

bool aes_bitblk_pixel(const struct aes_point p,
	const struct aes_bitblk *bitblk);

bool aes_palette_color(aes_id_t aes_id,
	const int index, struct vdi_color *color);

struct aes_area aes_objc_bounds(aes_id_t aes_id,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_);

static inline struct aes_point aes_point_add(
	const struct aes_point a,
	const struct aes_point b)
{
	return (struct aes_point) {
		.x = a.x + b.x,
		.y = a.y + b.y
	};
}

static inline struct aes_point aes_point_sub(
	const struct aes_point a,
	const struct aes_point b)
{
	return (struct aes_point) {
		.x = a.x - b.x,
		.y = a.y - b.y
	};
}

static inline struct aes_point aes_point_negate(struct aes_point p)
{
	return (struct aes_point) {
		.x = -p.x,
		.y = -p.y
	};
}

struct aes_object_shape_iterator {
	bool (*first)(struct aes_object_shape *shape,
		struct aes_object_shape_iterator *iterator);
	bool (*next)(struct aes_object_shape *shape,
		struct aes_object_shape_iterator *iterator);
	void *arg;
};

int aes_object_shape_pixel(aes_id_t aes_id, const struct aes_point p,
	const struct aes_object_shape *shape);

#endif /* _GEM_AES_H */
