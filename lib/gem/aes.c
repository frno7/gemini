// SPDX-License-Identifier: GPL-2.0

#include <stdlib.h>
#include <string.h>

#include <gem/aes.h>
#include <gem/aes-area.h>
#include <gem/aes-rsc.h>
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

static int aes_object_grid_to_x_px(const struct rsc_object_grid g,
	const struct aes_rectangle cb)
{
	return cb.w * g.ch + g.px;
}

static int aes_object_grid_to_y_px(const struct rsc_object_grid g,
	const struct aes_rectangle cb)
{
	return cb.h * g.ch + g.px;
}

struct rsc_object_grid rsc_object_grid_add(
	const struct rsc_object_grid a,
	const struct rsc_object_grid b)
{
	return (struct rsc_object_grid) {
		.px = a.px + b.px,
		.ch = a.ch + b.ch
	};
}

struct rsc_object_point rsc_object_point_add(
	const struct rsc_object_point a,
	const struct rsc_object_point b)
{
	return (struct rsc_object_point) {
		.x = rsc_object_grid_add(a.x, b.x),
		.y = rsc_object_grid_add(a.y, b.y)
	};
}

static struct rsc_object_point rsc_tree_object_origin(int16_t ob,
	const struct rsc_object *tree)
{
	BUG_ON(!rsc_valid_ob(ob));

	const int outline = tree->shape.state.outlined ? 3 : 0;
	struct rsc_object_point origin = {
		.x = { .px = outline },
		.y = { .px = outline }
	};

	/* Ignore root object position */
	for (; ob > 0; ob = rsc_object_parent(ob, tree))
		origin = rsc_object_point_add(origin, tree[ob].shape.area.p);

	return origin;
}

struct aes_object_border {
	int color;
	int thickness;
};

static struct aes_object_border aes_objc_border(const int ob,
	const struct rsc_object *tree, const struct rsc *rsc_)
{
	switch (tree[ob].shape.type.g) {
	case GEM_G_BOX:
	case GEM_G_BOXCHAR:
		return (struct aes_object_border) {
			.color = tree[ob].shape.spec.box.color.border,
			.thickness = tree[ob].shape.spec.box.thickness
		};
	case GEM_G_BUTTON:
		return (struct aes_object_border) {
			.color = 1,
			.thickness = (tree[ob].shape.flags.exit     ? -1 : 0) +
				     (tree[ob].shape.flags.default_ ? -1 : 0) - 1
		};
	}

	return (struct aes_object_border) { };
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

static struct aes_rectangle aes_grid(aes_id_t aes_id)
{
	const struct fnt *fnt_ = aes_fnt_large(aes_id);

	return (struct aes_rectangle) {
		.w = fnt_ ? fnt_->header->max_cell_width : 0,
		.h = fnt_ ? fnt_->header->bitmap_lines : 0
	};
}

bool aes_palette_color(aes_id_t aes_id,
	const int index, struct vdi_color *color)
{
	return vq_color(aes_id.aes_->vdi_id, index, color);
}

struct aes_area aes_objc_area(aes_id_t aes_id,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	const struct aes_rectangle grid = aes_grid(aes_id);
	const struct rsc_object_point p0 = rsc_tree_object_origin(ob, tree);
	const struct rsc_object_rectangle r0 = tree[ob].shape.area.r;

	return (struct aes_area) {
		.p = {
			.x = aes_object_grid_to_x_px(p0.x, grid),
			.y = aes_object_grid_to_y_px(p0.y, grid)
		},
		.r = {
			.w = aes_object_grid_to_x_px(r0.w, grid),
			.h = aes_object_grid_to_y_px(r0.h, grid)
		}
	};
}

struct aes_area aes_objc_bounds(aes_id_t aes_id,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	const int outline = tree[ob].shape.state.outlined ? -3 : 0;
	const int border = aes_objc_border(ob, tree, rsc_).thickness;
	const int resize = min(border, outline);

	return aes_area_shrink(aes_objc_area(aes_id, ob, tree, rsc_), resize);
}

static bool aes_area_within(const struct aes_point p,
	const struct aes_area area)
{
	return p.x >= area.p.x &&
	       p.y >= area.p.y &&
	       p.x <  area.p.x + area.r.w &&
	       p.y <  area.p.y + area.r.h;
}

static bool aes_objc_within_bounds(aes_id_t aes_id, const struct aes_point p,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	return aes_area_within(p, aes_objc_bounds(aes_id, ob, tree, rsc_));
}

static int aes_objc_find(aes_id_t aes_id, const struct aes_point p,
	const struct rsc_object *tree, const struct rsc *rsc_)
{
	int found = -1;

	for (int16_t ob = 0; rsc_valid_ob(ob); ob = rsc_tree_traverse(ob, tree))
		if (aes_objc_within_bounds(aes_id, p, ob, tree, rsc_))
			found = ob;

	return found;
}

static bool aes_area_outline(const struct aes_point p,
	const struct aes_area area)
{
	return !aes_area_within(p, aes_area_shrink(area, -2)) &&
		aes_area_within(p, aes_area_shrink(area, -3));
}

static bool aes_area_border(const struct aes_point p,
	const struct aes_area area, const int d)
{
	return !d     ? false :
		d < 0 ? !aes_area_within(p, area) &&
			 aes_area_within(p, aes_area_shrink(area, d)) :
			!aes_area_within(p, aes_area_shrink(area, d)) &&
			 aes_area_within(p, area);
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

	if (!fnt_ || !aes_area_within(p, text_area))
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

	if (!aes_area_within(p, bitblk_area))
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

		if (aes_area_within(p, char_area))
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

	if (aes_area_within(p, text_area))
		return aes_string_pixel(aes_id, p, text_area,
			iconblk->text.s,
			aes_area_justify_rectangle_center,
			aes_char_pixel, aes_fnt_small);

	if (!aes_area_within(p, icon_area))
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

int aes_objc_pixel(aes_id_t aes_id, const struct aes_point p,
	const struct rsc_object *tree, const struct rsc *rsc_)
{
	const int ob = aes_objc_find(aes_id, p, tree, rsc_);

	if (!rsc_valid_ob(ob))
		return -1;

	const struct aes_area area = aes_objc_area(aes_id, ob, tree, rsc_);
	const struct aes_object_border border = aes_objc_border(ob, tree, rsc_);

	if (aes_area_border(p, area, border.thickness))
		return border.color;

	if (tree[ob].shape.state.outlined && aes_area_outline(p, area))
		return border.color;

	const struct aes_object_shape shape =
		aes_rsc_object_shape(aes_id, area.p, &tree[ob], rsc_);

	switch (tree[ob].shape.type.g) {
#define AES_OBJECT_G_TYPE_SPEC(n_, symbol_, label_, spec_)		\
	case n_: return aes_g_ ## symbol_ ## _pixel(aes_id, p, &shape);
GEM_OBJECT_G_TYPE(AES_OBJECT_G_TYPE_SPEC)
	}

	return -1;
}
