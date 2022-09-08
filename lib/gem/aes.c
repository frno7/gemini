// SPDX-License-Identifier: GPL-2.0

#include <stdlib.h>
#include <string.h>

#include <gem/aes.h>
#include <gem/aes-area.h>
#include <gem/aes-shape.h>
#include <gem/vdi_.h>

#include "internal/assert.h"

aes_id_t aes_appl_init(struct aes *aes_)
{
	vdi_id_t vdi_id = vdi_v_opnwk(NULL, NULL);

	*aes_ = (struct aes) { .vdi_id = vdi_id };

	return (aes_id_t) { .aes_ = aes_ };
}

void aes_appl_exit(aes_id_t aes_id)
{
	vdi_v_clswk(aes_id.aes_->vdi_id);
}

struct aes_iconblk_pixel aes_iconblk_pixel(const struct aes_point p,
	const struct aes_iconblk *iconblk)
{
	if (p.x < 0 || p.y < 0 ||
	    p.x >= iconblk->bitmap.area.r.w ||
	    p.y >= iconblk->bitmap.area.r.h)
		return (struct aes_iconblk_pixel) { };

	const size_t offset = (p.x / 8) + (iconblk->bitmap.area.r.w / 8) * p.y;
	const uint8_t d = iconblk->bitmap.data[offset];
	const uint8_t m = iconblk->bitmap.mask[offset];
	const uint8_t w = d & (0x80 >> (p.x & 0x7));

	return (struct aes_iconblk_pixel) {
		.data = (d & w) ? 1 : 0,
		.mask = (m & w) ? 1 : 0,
	};
}

bool aes_bitblk_pixel(const struct aes_point p,
	const struct aes_bitblk *bitblk)
{
	if (p.x < 0 || p.y < 0 ||
	    p.x >= bitblk->area.r.w ||
	    p.y >= bitblk->area.r.h)
		return false;

	const size_t offset = (p.x / 8) + (bitblk->area.r.w / 8) * p.y;
	const uint8_t d = bitblk->data[offset];
	const uint8_t w = d & (0x80 >> (p.x & 0x7));

	return (d & w) != 0;
}

typedef struct fnt *(*aes_fnt_f)(aes_id_t aes_id);

struct fnt *aes_fnt_large(aes_id_t aes_id)
{
	return aes_id.aes_->vdi_id.vdi->font.large;
}

struct fnt *aes_fnt_small(aes_id_t aes_id)
{
	return aes_id.aes_->vdi_id.vdi->font.small;
}

bool aes_palette_color(aes_id_t aes_id,
	const int index, struct vdi_color *color)
{
	return vq_color(aes_id.aes_->vdi_id, index, color);
}

static bool aes_find_shape(struct aes_object_shape *shape,
	const struct aes_point p, struct aes_object_shape_iterator *iterator)
{
	struct aes_object_shape s;
	bool found = false;

	aes_for_each_object_shape (&s, iterator)
		if (aes_point_within_area(p, s.area)) {
			*shape = s;
			found = true;
		}

	return found;
}

typedef bool (*aes_char_pixel_f)(const struct aes_point p,
	const uint16_t c, const struct fnt *fnt);

static bool aes_char_pixel(const struct aes_point p,
	const uint16_t c, const struct fnt *fnt)
{
	return fnt_char_pixel(p.x, p.y, c, fnt);
}

static bool aes_char_pixel_lighten(const struct aes_point p,
	const uint16_t c, const struct fnt *fnt)
{
	return fnt_char_lighten(p.x, p.y, c, fnt) &&
	       fnt_char_pixel(p.x, p.y, c, fnt);
}

static bool aes_string_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_area area, const char *s,
	const aes_area_justify_rectangle_f justify_text,
	const aes_char_pixel_f char_pixel, const aes_fnt_f font)
{
	struct fnt *fnt_ = font(aes_id);
	const size_t length = strlen(s);
	const struct aes_rectangle grid = {
		.w = fnt_->header->max_cell_width,
		.h = fnt_->header->bitmap_lines
	};

	const struct aes_rectangle text_rectangle = {
		.w = grid.w * length,
		.h = grid.h
	};
	const struct aes_area text_area = justify_text(text_rectangle, area);

	if (!fnt_ || !aes_point_within_area(p, text_area))
		return false;

	const int i  = (p.x - text_area.p.x) / grid.w;

	if (i < 0 || i >= length)
		return false;

	const struct aes_point cp = {
		.x = (p.x - text_area.p.x) % grid.w,
		.y =  p.y - text_area.p.y
	};
	const char c = s[i];

	if (char_pixel(cp, c, fnt_))
		return true;

	return false;
}

static aes_area_justify_rectangle_f aes_tedinfo_justification(
	const struct aes_tedinfo *t)
{
	return t->just == aes_tedinfo_left  ?
				aes_area_justify_rectangle_center_left  :
	       t->just == aes_tedinfo_right ?
				aes_area_justify_rectangle_center_right :
				aes_area_justify_rectangle_center;
}

static int aes_g_box_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_object_shape *shape)
{
	static const uint16_t patterns[8][4] = {
		{ 0x0000, 0x0000, 0x0000, 0x0000 },
		{ 0x0000, 0x4444, 0x0000, 0x1111 },
		{ 0x0000, 0x5555, 0x0000, 0x5555 },
		{ 0x8888, 0x5555, 0x2222, 0x5555 },
		{ 0xAAAA, 0x5555, 0xAAAA, 0x5555 },
		{ 0xAAAA, 0xDDDD, 0xAAAA, 0x7777 },
		{ 0xAAAA, 0xFFFF, 0xAAAA, 0xFFFF },
		{ 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF }
	};

	return (patterns[shape->spec.box.color.pattern & 0x7][p.y & 0x3] &
		(0x8000 >> (p.x & 0xf))) ? shape->spec.box.color.fill : 0;
}

static int aes_g_boxchar_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_object_shape *shape)
{
	const char s[] = { shape->spec.box.c, '\0' };

	if (aes_string_pixel(aes_id, p, shape->area, s,
			aes_area_justify_rectangle_center,
				aes_char_pixel, aes_fnt_large))
		return !shape->state.selected;

	return aes_g_box_pixel(aes_id, p, shape);
}

static int aes_g_text_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_object_shape *shape)
{
	const struct aes_tedinfo *t = &shape->spec.tedinfo;

	return aes_string_pixel(aes_id, p, shape->area, t->text,
		aes_tedinfo_justification(t), aes_char_pixel,
		aes_fnt_large) ^ shape->state.selected;
}

static int aes_g_ftext_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_object_shape *shape)
{
	const struct aes_tedinfo *t = &shape->spec.tedinfo;

	return aes_string_pixel(aes_id, p, shape->area, t->tmplt,
		aes_tedinfo_justification(t), aes_char_pixel,
		aes_fnt_large) ^ shape->state.selected;
}

static int aes_g_string_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_object_shape *shape)
{
	return aes_string_pixel(aes_id, p, shape->area, shape->spec.string,
		aes_area_justify_rectangle_center_left,
		shape->state.disabled ? aes_char_pixel_lighten : aes_char_pixel,
		aes_fnt_large) ^ shape->state.selected;
}

static int aes_g_button_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_object_shape *shape)
{
	return aes_string_pixel(aes_id, p, shape->area, shape->spec.string,
		aes_area_justify_rectangle_center,
		aes_char_pixel, aes_fnt_large) ^ shape->state.selected;
}

static int aes_g_image_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_object_shape *shape)
{
	const struct aes_bitblk *bitblk = &shape->spec.bitblk;

	if (!bitblk)
		return 0;

	const struct aes_area bitblk_area =
		aes_area_justify_rectangle_center(
			bitblk->area.r, shape->area);

	if (!aes_point_within_area(p, bitblk_area))
		return 0;

	return aes_bitblk_pixel((struct aes_point) {
			p.x - bitblk->area.p.x,
			p.y - bitblk->area.p.y
		}, bitblk);
}

static int aes_g_icon_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_object_shape *shape)
{
	const struct aes_iconblk *iconblk = &shape->spec.iconblk;

	const struct aes_area icon_area =
		aes_area_justify_rectangle_top_center(
			iconblk->bitmap.area.r, shape->area);

	const struct fnt *fnt_small = aes_fnt_small(aes_id);

	if (fnt_small && iconblk->char_.c) {
		const struct aes_area char_area = (struct aes_area) {
			.p = {
				.x = icon_area.p.x + iconblk->char_.area.p.x,
				.y = icon_area.p.y + iconblk->char_.area.p.y
			},
			.r = {
				.w = fnt_small->header->max_cell_width,
				.h = fnt_small->header->bitmap_lines
			}
		};

		const char s[] = { iconblk->char_.c, '\0' };

		if (aes_point_within_area(p, char_area))
			return aes_string_pixel(aes_id, p, char_area,
				s, aes_area_justify_rectangle_center,
				aes_char_pixel, aes_fnt_small) ?
					iconblk->char_.color.fg :
					iconblk->char_.color.bg;
	}

	const struct aes_area text_area = (struct aes_area) {
		.p = {
			.x = shape->area.p.x + iconblk->text.area.p.x,
			.y = shape->area.p.y + iconblk->text.area.p.y
		},
		.r = {
			.w = iconblk->text.area.r.w,
			.h = iconblk->text.area.r.h
		}
	};

	if (aes_point_within_area(p, text_area))
		return aes_string_pixel(aes_id, p, text_area,
			iconblk->text.s,
			aes_area_justify_rectangle_center,
			aes_char_pixel, aes_fnt_small);

	if (!aes_point_within_area(p, icon_area))
		return 0;

	const struct aes_iconblk_pixel px = aes_iconblk_pixel(
		(struct aes_point) {
			.x = p.x - icon_area.p.x,
			.y = p.y - icon_area.p.y
		}, iconblk);

	if (!px.data && !px.mask)
		return 0;

	return px.data;
}

static int aes_g_cicon_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_object_shape *shape)
{
	return -1;	/* FIXME */
}

static int aes_g_title_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_object_shape *shape)
{
	return -1;	/* FIXME */
}

static int aes_g_ibox_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_object_shape *shape)
{
	return -1;	/* FIXME */
}

static int aes_g_progdef_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_object_shape *shape)
{
	return -1;	/* FIXME */
}

static int aes_g_boxtext_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_object_shape *shape)
{
	return -1;	/* FIXME */
}

static int aes_g_fboxtext_pixel(aes_id_t aes_id,
	const struct aes_point p, const struct aes_object_shape *shape)
{
	return -1;	/* FIXME */
}

int aes_object_shape_pixel(aes_id_t aes_id, const struct aes_point p,
	struct aes_object_shape_iterator *iterator)
{
	struct aes_object_simple_shape_iterator_arg simple_arg;
	struct aes_object_shape_iterator i =
		aes_object_simple_shape_iterator(iterator, &simple_arg);
	struct aes_object_shape shape;

	if (!aes_find_shape(&shape, p, &i))
		return -1;

	switch (shape.type.g) {
#define AES_OBJECT_G_TYPE_SPEC(n_, symbol_, label_, spec_)		\
	case n_: return aes_g_ ## symbol_ ## _pixel(aes_id, p, &shape);
GEM_OBJECT_G_TYPE(AES_OBJECT_G_TYPE_SPEC)
	}

	return -1;
}
