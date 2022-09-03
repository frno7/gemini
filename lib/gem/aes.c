// SPDX-License-Identifier: GPL-2.0

#include <stdlib.h>
#include <string.h>

#include <gem/aes.h>
#include <gem/vdi_.h>

#include "internal/assert.h"
#include "internal/compare.h"

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

	const int outline = tree->attr.state.outlined ? 3 : 0;
	struct rsc_object_point origin = {
		.x = { .px = outline },
		.y = { .px = outline }
	};

	/* Ignore root object position */
	for (; ob > 0; ob = rsc_object_parent(ob, tree))
		origin = rsc_object_point_add(origin, tree[ob].attr.area.p);

	return origin;
}

struct aes_object_border {
	int color;
	int thickness;
};

static struct aes_object_border aes_objc_border(const int ob,
	const struct rsc_object *tree, const struct rsc *rsc_)
{
	switch (tree[ob].attr.type.g) {
	case GEM_G_BOX:
	case GEM_G_BOXCHAR:
		return (struct aes_object_border) {
			.color = tree[ob].attr.spec.box.color.border,
			.thickness = tree[ob].attr.spec.box.thickness
		};
	case GEM_G_BUTTON:
		return (struct aes_object_border) {
			.color = 1,
			.thickness = (tree[ob].attr.flags.exit     ? -1 : 0) +
				     (tree[ob].attr.flags.default_ ? -1 : 0) - 1
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

typedef struct aes_area (*justification_f)(
	const struct aes_rectangle rectangle,
	const struct aes_area area);

static struct aes_area justify_top_center(
	const struct aes_rectangle rectangle,
	const struct aes_area area)
{
	return (struct aes_area) {
		.p = {
			.x = area.p.x + (area.r.w - rectangle.w) / 2,
			.y = area.p.y
		},
		.r = rectangle
	};
}

static struct aes_area justify_center_left(
	const struct aes_rectangle rectangle,
	const struct aes_area area)
{
	return (struct aes_area) {
		.p = {
			.x = area.p.x,
			.y = area.p.y + (area.r.h - rectangle.h) / 2
		},
		.r = rectangle
	};
}

static struct aes_area justify_center_center(
	const struct aes_rectangle rectangle,
	const struct aes_area area)
{
	return (struct aes_area) {
		.p = {
			.x = area.p.x + (area.r.w - rectangle.w) / 2,
			.y = area.p.y + (area.r.h - rectangle.h) / 2
		},
		.r = rectangle
	};
}

static struct aes_area justify_center_right(
	const struct aes_rectangle rectangle,
	const struct aes_area area)
{
	return (struct aes_area) {
		.p = {
			.x = area.p.x + (area.r.w - rectangle.w),
			.y = area.p.y + (area.r.h - rectangle.h) / 2
		},
		.r = rectangle
	};
}

static struct aes_area aes_area_shrink(
	const struct aes_area area, const int d)
{
	return justify_center_center(
		(struct aes_rectangle) {
			.w = max(0, area.r.w - 2 * d),
			.h = max(0, area.r.h - 2 * d)
		}, area);
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
	const struct rsc_object_rectangle r0 = tree[ob].attr.area.r;

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
	const int outline = tree[ob].attr.state.outlined ? -3 : 0;
	const int border = aes_objc_border(ob, tree, rsc_).thickness;
	const int resize = min(border, outline);

	return aes_area_shrink(aes_objc_area(aes_id, ob, tree, rsc_), resize);
}

static bool aes_area_within(const int x, const int y,
	const struct aes_area area)
{
	return x >= area.p.x &&
	       y >= area.p.y &&
	       x <  area.p.x + area.r.w &&
	       y <  area.p.y + area.r.h;
}

static bool aes_objc_within_bounds(aes_id_t aes_id, const int x, const int y,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	return aes_area_within(x, y, aes_objc_bounds(aes_id, ob, tree, rsc_));
}

static int aes_objc_find(aes_id_t aes_id, const int x, const int y,
	const struct rsc_object *tree, const struct rsc *rsc_)
{
	int found = -1;

	for (int16_t ob = 0; rsc_valid_ob(ob); ob = rsc_tree_traverse(ob, tree))
		if (aes_objc_within_bounds(aes_id, x, y, ob, tree, rsc_))
			found = ob;

	return found;
}

static bool aes_area_outline(const int x, const int y,
	const struct aes_area area)
{
	return !aes_area_within(x, y, aes_area_shrink(area, -2)) &&
		aes_area_within(x, y, aes_area_shrink(area, -3));
}

static bool aes_area_border(const int x, const int y,
	const struct aes_area area, const int d)
{
	return !d     ? false :
		d < 0 ? !aes_area_within(x, y, area) &&
			 aes_area_within(x, y, aes_area_shrink(area, d)) :
			!aes_area_within(x, y, aes_area_shrink(area, d)) &&
			 aes_area_within(x, y, area);
}

typedef bool (*aes_char_pixel_f)(const int x, const int y,
	const uint16_t c, const struct fnt *fnt);

static bool aes_char_pixel(const int x, const int y,
	const uint16_t c, const struct fnt *fnt)
{
	return fnt_char_pixel(x, y, c, fnt);
}

static bool aes_char_pixel_lighten(const int x, const int y,
	const uint16_t c, const struct fnt *fnt)
{
	return fnt_char_lighten(x, y, c, fnt) &&
	       fnt_char_pixel(x, y, c, fnt);
}

static bool aes_string_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area, const char *s,
	const justification_f justification, const aes_char_pixel_f char_pixel,
	const aes_fnt_f font)
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
	const struct aes_area text_area = justification(text_rectangle, area);

	if (!fnt_ || !aes_area_within(x, y, text_area))
		return false;

	const int i  = (x - text_area.p.x) / grid.w;

	if (i < 0 || i >= length)
		return false;

	const int cx = (x - text_area.p.x) % grid.w;
	const int cy =  y - text_area.p.y;
	const unsigned char c = s[i];

	if (char_pixel(cx, cy, c, fnt_))
		return true;

	return false;
}

static justification_f rsc_tedinfo_justification(const struct rsc_tedinfo *t)
{
	return t->te_just == rsc_tedinfo_left  ? justify_center_left  :
	       t->te_just == rsc_tedinfo_right ? justify_center_right :
						 justify_center_center;
}

static int aes_g_box_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
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

	return (patterns[tree[ob].attr.spec.box.color.pattern & 0x7][y & 0x3] &
		(0x8000 >> (x & 0xf))) ? tree[ob].attr.spec.box.color.fill : 0;
}

static int aes_g_boxchar_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	const char s[] = { tree[ob].attr.spec.box.c, '\0' };

	if (aes_string_pixel(aes_id, x, y, area, s,
			justify_center_center, aes_char_pixel, aes_fnt_large))
		return !tree[ob].attr.state.selected;

	return aes_g_box_pixel(aes_id, x, y, area, ob, tree, rsc_);
}

static int aes_g_text_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	const struct rsc_tedinfo *t =
		rsc_tedinfo_at_offset(tree[ob].attr.spec.tedinfo, rsc_);

	return aes_string_pixel(aes_id, x, y, area,
		rsc_string_at_offset(t->te_text, rsc_),
		rsc_tedinfo_justification(t), aes_char_pixel,
		aes_fnt_large) ^ tree[ob].attr.state.selected;
}

static int aes_g_ftext_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	const struct rsc_tedinfo *t =
		rsc_tedinfo_at_offset(tree[ob].attr.spec.tedinfo, rsc_);

	return aes_string_pixel(aes_id, x, y, area,
		rsc_string_at_offset(t->te_tmplt, rsc_),
		rsc_tedinfo_justification(t), aes_char_pixel,
		aes_fnt_large) ^ tree[ob].attr.state.selected;
}

static int aes_g_string_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	return aes_string_pixel(aes_id, x, y, area,
		rsc_string_at_offset(tree[ob].attr.spec.string, rsc_),
		justify_center_left, tree[ob].attr.state.disabled ?
			aes_char_pixel_lighten : aes_char_pixel,
		aes_fnt_large) ^
		tree[ob].attr.state.selected;
}

static int aes_g_button_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	return aes_string_pixel(aes_id, x, y, area,
		rsc_string_at_offset(tree[ob].attr.spec.string, rsc_),
		justify_center_center, aes_char_pixel, aes_fnt_large) ^
		tree[ob].attr.state.selected;
}

static int aes_g_image_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	const struct rsc_bitblk *bitblk =
		rsc_bitblk_at_offset(tree[ob].attr.spec.bitblk, rsc_);

	if (!bitblk)
		return 0;

	const struct aes_area bitblk_area = justify_center_center(
		(struct aes_rectangle) {
			.w = bitblk->bi_wb * 8,
			.h = bitblk->bi_hl
		}, area);

	if (!aes_area_within(x, y, bitblk_area))
		return 0;

	return rsc_bitblk_pixel(
		x - bitblk_area.p.x,
		y - bitblk_area.p.y, bitblk, rsc_);
}

static int aes_g_icon_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	const struct rsc_iconblk *iconblk =
		rsc_iconblk_at_offset(tree[ob].attr.spec.iconblk, rsc_);

	if (!iconblk)
		return 0;

	const struct aes_area icon_area = justify_top_center(
		(struct aes_rectangle) {
			.w = iconblk->ib_icon.r.w,
			.h = iconblk->ib_icon.r.h
		}, area);

	const struct fnt *fnt_small = aes_fnt_small(aes_id);

	if (fnt_small && iconblk->ib_char.c) {
		const struct aes_area char_area = (struct aes_area) {
			.p = {
				.x = icon_area.p.x + iconblk->ib_char.p.x,
				.y = icon_area.p.y + iconblk->ib_char.p.y
			},
			.r = {
				.w = fnt_small->header->max_cell_width,
				.h = fnt_small->header->bitmap_lines
			}
		};

		const char s[] = { iconblk->ib_char.c, '\0' };

		if (aes_area_within(x, y, char_area))
			return aes_string_pixel(aes_id, x, y, char_area,
				s, justify_center_center, aes_char_pixel,
				aes_fnt_small) ?
					iconblk->ib_char.color.fg :
					iconblk->ib_char.color.bg;
	}

	const struct aes_area text_area = (struct aes_area) {
		.p = {
			.x = area.p.x + iconblk->ib_text.p.x,
			.y = area.p.y + iconblk->ib_text.p.y
		},
		.r = {
			.w = iconblk->ib_text.r.w,
			.h = iconblk->ib_text.r.h
		}
	};

	if (aes_area_within(x, y, text_area))
		return aes_string_pixel(aes_id, x, y, text_area,
			rsc_string_at_offset(iconblk->ib_ptext, rsc_),
			justify_center_center, aes_char_pixel, aes_fnt_small);

	if (!aes_area_within(x, y, icon_area))
		return 0;

	const struct rsc_iconblk_pixel p = rsc_iconblk_pixel(
		x - icon_area.p.x,
		y - icon_area.p.y, iconblk, rsc_);

	if (!p.data && !p.mask)
		return 0;

	return p.data;
}

static int aes_g_cicon_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	return -1;	/* FIXME */
}

static int aes_g_title_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	return -1;	/* FIXME */
}

static int aes_g_ibox_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	return -1;	/* FIXME */
}

static int aes_g_progdef_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	return -1;	/* FIXME */
}

static int aes_g_boxtext_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	return -1;	/* FIXME */
}

static int aes_g_fboxtext_pixel(aes_id_t aes_id,
	const int x, const int y, const struct aes_area area,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_)
{
	return -1;	/* FIXME */
}

int aes_objc_pixel(aes_id_t aes_id, const int x, const int y,
	const struct rsc_object *tree, const struct rsc *rsc_)
{
	const int ob = aes_objc_find(aes_id, x, y, tree, rsc_);

	if (!rsc_valid_ob(ob))
		return -1;

	const struct aes_area area = aes_objc_area(aes_id, ob, tree, rsc_);
	const struct aes_object_border border = aes_objc_border(ob, tree, rsc_);

	if (aes_area_border(x, y, area, border.thickness))
		return border.color;

	if (tree[ob].attr.state.outlined && aes_area_outline(x, y, area))
		return border.color;

	switch (tree[ob].attr.type.g) {
#define RSC_OBJECT_G_TYPE_SPEC(n_, symbol_, label_, spec_)		\
	case n_: return aes_g_ ## symbol_ ## _pixel(			\
		aes_id, x, y, area, ob, tree, rsc_);
GEM_OBJECT_G_TYPE(RSC_OBJECT_G_TYPE_SPEC)
	}

	return -1;
}
