// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#include <gem/aes-area.h>
#include <gem/aes-rsc.h>
#include <gem/aes-shape.h>

static struct aes_rectangle aes_rsc_grid(aes_id_t aes_id)
{
	const struct fnt *fnt = aes_fnt_large(aes_id);

	return (struct aes_rectangle) {
		.w = fnt ? fnt->header->max_cell_width : 0,
		.h = fnt ? fnt->header->bitmap_lines : 0
	};
}

static struct aes_object_flags aes_rsc_object_shape_flags(
	struct rsc_object_flags flags)
{
	return (struct aes_object_flags) {
#define RSC_OBJECT_FLAG_COPY(bit_, symbol_, label_)			\
		.symbol_ = flags.symbol_,
GEM_OBJECT_FLAG(RSC_OBJECT_FLAG_COPY)
	};
}

static struct aes_object_state aes_rsc_object_shape_state(
	struct rsc_object_state state)
{
	return (struct aes_object_state) {
#define RSC_OBJECT_STATE_COPY(bit_, symbol_, label_)			\
		.symbol_ = state.symbol_,
GEM_OBJECT_STATE(RSC_OBJECT_STATE_COPY)
	};
}

static struct aes_area aes_rsc_object_shape_area(aes_id_t aes_id,
	const struct aes_point p, const struct rsc_object_rectangle r)
{
	const struct aes_rectangle gr = aes_rsc_grid(aes_id);

	return (struct aes_area) {
		.p = p,
		.r = {
			.w = gr.w * r.w.ch + r.w.px,
			.h = gr.h * r.h.ch + r.h.px
		}
	};
}

static struct aes_point aes_point_from_rcs(
	aes_id_t aes_id, const struct rsc_object_point p)
{
	const struct aes_rectangle gr = aes_rsc_grid(aes_id);

	return (struct aes_point) {
		.x = gr.w * p.x.ch + p.x.px,
		.y = gr.h * p.y.ch + p.y.px
	};
}

static struct aes_object_color aes_rsc_object_color(
	const struct rsc_object_color color)
{
	return (struct aes_object_color) {
		.border  = color.border,
		.text    = color.text,
		.opaque  = color.opaque,
		.pattern = color.pattern,
		.fill    = color.fill
	};
}

static struct aes_object_spec aes_rsc_object_shape_spec_box(
	aes_id_t aes_id, struct rsc_object_spec spec, const struct rsc *rsc)
{
	const struct rsc_object_spec_box box = spec.box;

	return (struct aes_object_spec) {
		.box = {
			.c = box.c,
			.thickness = box.thickness,
			.color = aes_rsc_object_color(box.color)
		}
	};
}

static char *aes_rsc_object_shape_spec_tedinfo_string(
	const uint32_t offset, const struct rsc *rsc)
{
	return rsc_string_at_offset(offset, rsc);
}

static int aes_rsc_object_shape_spec_tedinfo_font(
	const uint16_t font, const struct rsc *rsc)
{
	return font;
}

static int aes_rsc_object_shape_spec_tedinfo_just(
	const uint16_t just, const struct rsc *rsc)
{
	return just;
}

static int aes_rsc_object_shape_spec_tedinfo_integer(
	const uint16_t integer, const struct rsc *rsc)
{
	return integer;
}

static struct aes_object_color aes_rsc_object_shape_spec_tedinfo_color(
	const struct rsc_object_color color, const struct rsc *rsc)
{
	return aes_rsc_object_color(color);
}

static struct aes_object_spec aes_rsc_object_shape_spec_tedinfo(
	aes_id_t aes_id, struct rsc_object_spec spec, const struct rsc *rsc)
{
	const struct rsc_tedinfo *rt = rsc_tedinfo_at_offset(spec.tedinfo, rsc);

	return (struct aes_object_spec) {
		.tedinfo = {
#define AES_TEDINFO_COPY(type_, name_, form_)				\
	.name_ = aes_rsc_object_shape_spec_tedinfo_ ## form_(		\
		rt->te_ ## name_, rsc),
AES_TEDINFO_FIELD(AES_TEDINFO_COPY)
		}
	};
}

static struct aes_object_spec aes_rsc_object_shape_spec_bitblk(
	aes_id_t aes_id, struct rsc_object_spec spec, const struct rsc *rsc)
{
	const struct rsc_bitblk *rb = rsc_bitblk_at_offset(spec.bitblk, rsc);

	return (struct aes_object_spec) {
		.bitblk = {
			.data = rsc_bitmap_at_offset(rb->bi_data, rsc),
			.area = {
				.p = {
					.x = rb->bi_x,
					.y = rb->bi_y
				},
				.r = {
					.w = rb->bi_wb * 8,
					.h = rb->bi_hl
				}
			},
			.color = rb->bi_color
		}
	};
}

static struct aes_object_spec aes_rsc_object_shape_spec_applblk(
	aes_id_t aes_id, struct rsc_object_spec spec, const struct rsc *rsc)
{
	return (struct aes_object_spec) { };	/* FIXME */
}

static struct aes_object_spec aes_rsc_object_shape_spec_string(
	aes_id_t aes_id, struct rsc_object_spec spec, const struct rsc *rsc)
{
	return (struct aes_object_spec) {
		.string = rsc_string_at_offset(spec.string, rsc)
	};
}

static struct aes_point aes_rsc_iconblk_point(struct rsc_iconblk_point p)
{
	return (struct aes_point) {
		.x = p.x,
		.y = p.y
	};
}

static struct aes_rectangle aes_rsc_iconblk_rectangle(
	struct rsc_iconblk_rectangle r)
{
	return (struct aes_rectangle) {
		.w = r.w,
		.h = r.h
	};
}

static struct aes_area aes_rsc_iconblk_area(struct rsc_iconblk_area area)
{
	return (struct aes_area) {
		.p = aes_rsc_iconblk_point(area.p),
		.r = aes_rsc_iconblk_rectangle(area.r)
	};
}

static struct aes_object_spec aes_rsc_object_shape_spec_iconblk(
	aes_id_t aes_id, struct rsc_object_spec spec, const struct rsc *rsc)
{
	const struct rsc_iconblk *ri = rsc_iconblk_at_offset(spec.iconblk, rsc);
	const struct fnt *fnt_small = aes_fnt_small(aes_id);
	const struct aes_rectangle char_r = fnt_small && ri->ib_char.c ?
		(struct aes_rectangle) {
			.w = fnt_small->header->max_cell_width,
			.h = fnt_small->header->bitmap_lines
		} :
		(struct aes_rectangle) { };

	return (struct aes_object_spec) {
		.iconblk = {
			.bitmap = {
				.data = rsc_bitmap_at_offset(ri->ib_data, rsc),
				.mask = rsc_bitmap_at_offset(ri->ib_mask, rsc),
				.area = aes_rsc_iconblk_area(ri->ib_icon)
			},
			.text = {
				.s = rsc_string_at_offset(ri->ib_text, rsc),
				.area = aes_rsc_iconblk_area(ri->ib_txt)
			},
			.char_ = {
				.c = ri->ib_char.c,
				.color = {
					.fg = ri->ib_char.color.fg,
					.bg = ri->ib_char.color.bg
				},
				.area = {
					.p = aes_rsc_iconblk_point(ri->ib_char.p),
					.r = char_r
				},
			},
		}
	};
}

static struct aes_object_spec aes_rsc_object_shape_spec_ciconblk(
	aes_id_t aes_id, struct rsc_object_spec spec, const struct rsc *rsc)
{
	return (struct aes_object_spec) { };	/* FIXME */
}

static struct aes_object_spec aes_rsc_object_shape_spec(aes_id_t aes_id,
	const struct rsc_object *ro, const struct rsc *rsc)
{
	switch (ro->shape.type.g) {
#define RSC_OBJECT_G_TYPE_SPEC(n_, symbol_, label_, spec_)		\
	case n_: return aes_rsc_object_shape_spec_ ## spec_(		\
		aes_id, ro->shape.spec, rsc);
GEM_OBJECT_G_TYPE(RSC_OBJECT_G_TYPE_SPEC)
	default: return (struct aes_object_spec) { };
	}
}

struct aes_object_shape aes_rsc_object_shape(aes_id_t aes_id,
	const struct aes_point p, const struct rsc_object *ro,
	const struct rsc *rsc)
{
	return (struct aes_object_shape) {
		.type  = { .g = ro->shape.type.g },
		.flags = aes_rsc_object_shape_flags(ro->shape.flags),
		.state = aes_rsc_object_shape_state(ro->shape.state),
		.spec  = aes_rsc_object_shape_spec(aes_id, ro, rsc),
		.area  = aes_rsc_object_shape_area(aes_id, p, ro->shape.area.r)
	};
}

int16_t aes_rsc_tree_traverse_with_origin(aes_id_t aes_id,
	struct aes_point *origin, int16_t ob, const struct rsc_object *tree)
{
	if (rsc_valid_ob(tree[ob].link.head)) {
		*origin = aes_point_add(*origin, aes_point_from_rcs(
			aes_id, tree[tree[ob].link.head].shape.area.p));

		return tree[ob].link.head;	/* Advance to the child */
	}

	if (ob)
		*origin = aes_point_sub(*origin, aes_point_from_rcs(
			aes_id, tree[ob].shape.area.p));

	for (;;) {
		const int16_t nx = tree[ob].link.next;

		if (!rsc_valid_ob(nx))
			return nx;		/* Unable to advance */

		if (ob != tree[nx].link.tail) {
			*origin = aes_point_add(*origin, aes_point_from_rcs(
				aes_id, tree[nx].shape.area.p));

			return nx;		/* Advance to the sibling */
		}

		ob = nx;			/* Advance to the parent */

		if (ob)
			*origin = aes_point_sub(*origin, aes_point_from_rcs(
				aes_id, tree[nx].shape.area.p));
	}
}

struct aes_area aes_rsc_tree_bounds(aes_id_t aes_id,
	const struct rsc_object *tree, const struct rsc *rsc)
{
	const struct aes_object_shape shape = aes_rsc_object_shape(aes_id,
		(struct aes_point) { }, &tree[0], rsc);
	struct aes_object_shape simple;
	struct aes_area bounds = { };
	int i = 0;

	aes_for_each_simple_object_shape (&simple, shape)
		bounds = !i++ ? simple.area :
			aes_area_bounds(bounds, simple.area);

	return bounds;
}
