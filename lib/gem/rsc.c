// SPDX-License-Identifier: GPL-2.0

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <gem/rsc.h>
#include <gem/rsc-map.h>

#include "internal/assert.h"
#include "internal/compare.h"
#include "internal/print.h"

static bool __attribute__((format(printf, 3, 4))) rsc_error(void *arg,
	const struct rsc_diagnostic *diagnostic, char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	report_msg(diagnostic->error, arg, "", "", fmt, ap);
	va_end(ap);

	return false;
}

size_t rsc_string_offset_at_index(const size_t i, const struct rsc *rsc)
{
	const size_t unextended_size = rsc_unextended_size(rsc);

	if (rsc->header->rsh_frstr + sizeof(uint32_t[i + 1]) > unextended_size)
		return 0;

	const uint8_t *b = (const uint8_t *)rsc->header;
	const struct rsc_string_table {
		uint32_t offset;
	} BE_STORAGE PACKED *a =
		(const struct rsc_string_table *)&b[rsc->header->rsh_frstr];

	return a[i].offset < unextended_size ? a[i].offset : 0;
}

char *rsc_string_at_offset(const size_t offset, const struct rsc *rsc)
{
	const size_t unextended_size = rsc_unextended_size(rsc);

	if (!offset || offset >= unextended_size)
		return NULL;

	char *c = (char *)rsc->header;

	for (size_t i = 0; offset + i < unextended_size; i++)
		if (!c[offset + i])
			return &c[offset];

	return NULL;
}

char *rsc_string_at_index(const size_t i, const struct rsc *rsc)
{
	return rsc_string_at_offset(rsc_string_offset_at_index(i, rsc), rsc);
}

struct rsc_tedinfo *rsc_tedinfo_at_offset(const size_t offset,
	const struct rsc *rsc)
{
	if (!offset ||
	    offset + sizeof(struct rsc_tedinfo) > rsc_unextended_size(rsc))
		return (struct rsc_tedinfo *)NULL;

	const uint8_t *b = (const uint8_t *)rsc->header;

	return (struct rsc_tedinfo *)&b[offset];
}

struct rsc_bitblk *rsc_bitblk_at_offset(const size_t offset,
	const struct rsc *rsc)
{
	if (!offset ||
	    offset + sizeof(struct rsc_bitblk) > rsc_unextended_size(rsc))
		return (struct rsc_bitblk *)NULL;

	const uint8_t *b = (const uint8_t *)rsc->header;

	return (struct rsc_bitblk *)&b[offset];
}

size_t rsc_frimg_offset_at_index(const size_t i, const struct rsc *rsc)
{
	const size_t unextended_size = rsc_unextended_size(rsc);

	if (i >= rsc->header->rsh_nimages ||
	    rsc->header->rsh_frimg + sizeof(uint32_t[i + 1]) > unextended_size)
		return 0;

	const uint8_t *b = (const uint8_t *)rsc->header;
	const struct rsc_frimg_table {
		uint32_t offset;
	} BE_STORAGE PACKED *a =
		(const struct rsc_frimg_table *)&b[rsc->header->rsh_frimg];

	return a[i].offset < unextended_size ? a[i].offset : 0;
}

struct rsc_bitblk *rsc_frimg_at_index(const size_t i, const struct rsc *rsc)
{
	return rsc_bitblk_at_offset(rsc_frimg_offset_at_index(i, rsc), rsc);
}

size_t rsc_iconblk_bitmap_size(const struct rsc_iconblk *iconblk)
{
	return (size_t)(iconblk->ib_icon.r.w / 8) *
	       (size_t)iconblk->ib_icon.r.h;
}

struct rsc_iconblk_pixel rsc_iconblk_pixel(int x, int y,
	const struct rsc_iconblk *iconblk, const struct rsc *rsc)
{
	const size_t data_offset = iconblk->ib_data + (x / 8) + (iconblk->ib_icon.r.w / 8) * y;
	const size_t mask_offset = iconblk->ib_mask + (x / 8) + (iconblk->ib_icon.r.w / 8) * y;
	const uint8_t *b = (const uint8_t *)rsc->header;

	if (x < 0 || y < 0 ||
	    x >= iconblk->ib_icon.r.w ||
	    y >= iconblk->ib_icon.r.h ||
	    data_offset >= rsc_unextended_size(rsc) ||
	    mask_offset >= rsc_unextended_size(rsc))
		return (struct rsc_iconblk_pixel) { };

	return (struct rsc_iconblk_pixel) {
		.data = (b[data_offset] & (0x80 >> (x % 8))) ? 1 : 0,
		.mask = (b[mask_offset] & (0x80 >> (x % 8))) ? 1 : 0,
	};
}

uint8_t *rsc_bitmap_at_offset(const size_t offset, const struct rsc *rsc)
{
	if (!offset || offset > rsc_unextended_size(rsc))
		return NULL;

	uint8_t *b = (uint8_t *)rsc->header;

	return &b[offset];
}

size_t rsc_bitblk_bitmap_size(const struct rsc_bitblk *bitblk)
{
	return (size_t)bitblk->bi_wb * (size_t)bitblk->bi_hl;
}

bool rsc_bitblk_pixel(int x, int y,
	const struct rsc_bitblk *bitblk, const struct rsc *rsc)
{
	const size_t offset = bitblk->bi_data + (x / 8) + bitblk->bi_wb * y;
	const uint8_t *b = (const uint8_t *)rsc->header;

	if (x < 0 || y < 0 ||
	    x >= bitblk->bi_wb * 8 ||
	    y >= bitblk->bi_hl ||
	    offset >= rsc_unextended_size(rsc))
		return false;

	return (b[offset] & (0x80 >> (x % 8))) ? true : false;
}

struct rsc_iconblk *rsc_iconblk_at_offset(const size_t offset,
	const struct rsc *rsc)
{
	if (offset + sizeof(struct rsc_object) > rsc_unextended_size(rsc))
		return (struct rsc_iconblk *)NULL;

	const uint8_t *b = (const uint8_t *)rsc->header;

	return (struct rsc_iconblk *)&b[offset];
}

size_t rsc_iconblk_offset_at_index(const size_t i, const struct rsc *rsc)
{
	const size_t unextended_size = rsc_unextended_size(rsc);

	if (i >= rsc->header->rsh_nib ||
	    rsc->header->rsh_iconblk + sizeof(uint32_t[i + 1]) > unextended_size)
		return 0;

	const uint8_t *b = (const uint8_t *)rsc->header;
	const struct rsc_iconblk_table {
		uint32_t offset;
	} BE_STORAGE PACKED *a =
		(const struct rsc_iconblk_table *)&b[rsc->header->rsh_iconblk];

	return a[i].offset < unextended_size ? a[i].offset : 0;
}

struct rsc_iconblk *rsc_iconblk_at_index(const size_t i, const struct rsc *rsc)
{
	return rsc_iconblk_at_offset(rsc_iconblk_offset_at_index(i, rsc), rsc);
}

size_t rsc_unextended_size(const struct rsc *rsc)
{
	const size_t size = min_t(size_t, rsc->size, rsc->header->rsh_rssize);

	return size < sizeof(*rsc->header) ? 0 : size;
}

struct rsc_object *rsc_tree_object_at_offset(
	const size_t offset, const struct rsc *rsc)
{
	if (!offset)
		return (struct rsc_object *)NULL;

	const uint8_t *b = (const uint8_t *)rsc->header;

	if (rsc_unextended_size(rsc) < offset + sizeof(struct rsc_object))
		return (struct rsc_object *)NULL;

	return (struct rsc_object *)&b[offset];
}

size_t rsc_tree_offset_at_index(const size_t i, const struct rsc *rsc)
{
	if (rsc->header->rsh_trindex < sizeof(*rsc->header))
		return 0;

	if (rsc->header->rsh_trindex + sizeof(uint32_t[i + 1]) >
			rsc_unextended_size(rsc))
		return 0;

	const uint8_t *b = (const uint8_t *)rsc->header;
	const struct rsc_tree_table {
		uint32_t offset;
	} BE_STORAGE PACKED *table =
		(const struct rsc_tree_table *)&b[rsc->header->rsh_trindex];

	if (rsc_unextended_size(rsc) < table[i].offset + sizeof(struct rsc_object))
		return 0;

	return table[i].offset;
}

struct rsc_object *rsc_tree_at_index(const size_t i, const struct rsc *rsc)
{
	return rsc_tree_object_at_offset(rsc_tree_offset_at_index(i, rsc), rsc);
}

int16_t rsc_object_parent(int16_t ob, const struct rsc_object *tree)
{
	BUG_ON(!rsc_valid_ob(ob));

	for (;;) {
		const int16_t nx = tree[ob].link.next;

		if (!rsc_valid_ob(nx))
			return nx;		/* There is no parent */

		if (ob == tree[nx].link.tail)
			return nx;		/* Parent found */

		ob = nx;			/* Advance to sibling */
	}
}

int16_t rsc_tree_traverse(int16_t ob, const struct rsc_object *tree)
{
	BUG_ON(!rsc_valid_ob(ob));

	if (rsc_valid_ob(tree[ob].link.head))
		return tree[ob].link.head;	/* Advance to the child */

	for (;;) {
		const int16_t nx = tree[ob].link.next;

		if (!rsc_valid_ob(nx))
			return nx;		/* Unable to advance */

		if (ob != tree[nx].link.tail)
			return nx;		/* Advance to the sibling */

		ob = nx;			/* Advance to the parent */
	}
}

static bool rsc_valid_header(const struct rsc *rsc)
{
	return rsc->size >= sizeof(*rsc->header);
}

static bool rsc_valid_header_vrsn(const struct rsc *rsc,
	const struct rsc_diagnostic *diagnostic, void *arg)
{
	const bool valid = rsc->header->rsh_vrsn <= 0x7;

	if (!valid)
		return rsc_error(arg, diagnostic,
			"Invalid header version %x", rsc->header->rsh_vrsn);
	else if (!valid)
		return rsc_error(arg, diagnostic, "Invalid header version");

	return valid;
}

static bool rsc_valid_header_rssize(const struct rsc *rsc,
	const struct rsc_diagnostic *diagnostic, void *arg)
{
	const bool valid = rsc->header->rsh_rssize >= sizeof(*rsc->header) &&
			   rsc->header->rsh_rssize <= rsc->size;

	if (!valid)
		return rsc_error(arg, diagnostic,
			"Invalid header size %u", rsc->header->rsh_rssize);
	else if (!valid)
		return rsc_error(arg, diagnostic, "Invalid header size");

	return valid;
}

bool rsc_valid_structure_diagnostic(const struct rsc *rsc,
	const struct rsc_diagnostic *diagnostic, void *arg)
{
	BUILD_BUG_ON(sizeof(struct rsc_header)             != 36);
	BUILD_BUG_ON(sizeof(struct rsc_object_color)       !=  2);
	BUILD_BUG_ON(sizeof(struct rsc_object_spec)        !=  4);
	BUILD_BUG_ON(sizeof(struct rsc_object_type)        !=  2);
	BUILD_BUG_ON(sizeof(struct rsc_object_flags)       !=  2);
	BUILD_BUG_ON(sizeof(struct rsc_object_state)       !=  2);
	BUILD_BUG_ON(sizeof(struct rsc_object_grid)        !=  2);
	BUILD_BUG_ON(sizeof(struct rsc_object_point)       !=  4);
	BUILD_BUG_ON(sizeof(struct rsc_object_rectangle)   !=  4);
	BUILD_BUG_ON(sizeof(struct rsc_object_area)        !=  8);
	BUILD_BUG_ON(sizeof(struct rsc_object)             != 24);
	BUILD_BUG_ON(sizeof(struct rsc_tedinfo)            != 28);
	BUILD_BUG_ON(sizeof(struct rsc_bitblk)             != 14);
	BUILD_BUG_ON(sizeof(struct rsc_iconblk_char_color) !=  1);
	BUILD_BUG_ON(sizeof(struct rsc_iconblk_point)      !=  4);
	BUILD_BUG_ON(sizeof(struct rsc_iconblk_char)       !=  6);
	BUILD_BUG_ON(sizeof(struct rsc_iconblk_rectangle)  !=  4);
	BUILD_BUG_ON(sizeof(struct rsc_iconblk_area)       !=  8);
	BUILD_BUG_ON(sizeof(struct rsc_iconblk)            != 34);

	if (!rsc_valid_header(rsc))
		return rsc_error(arg, diagnostic, "Malformed header");

	if (!rsc_valid_header_vrsn(rsc, diagnostic, arg))
		return false;

	if (!rsc_valid_header_rssize(rsc, diagnostic, arg))
		return false;

	if (!rsc_valid_map(rsc, diagnostic, arg))
		return rsc_error(arg, diagnostic, "Malformed structure");

	return true;
}

bool rsc_valid_structure(const struct rsc *rsc)
{
	const struct rsc_diagnostic diagnostic = { };

	return rsc_valid_structure_diagnostic(rsc, &diagnostic, NULL);
}
