// SPDX-License-Identifier: GPL-2.0

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <gem/rsc.h>
#include <gem/rsc-map.h>

#include "internal/print.h"

struct rsc_map_diagnostic {
	struct rsc_map *map;
	const struct rsc *rsc;

	const struct rsc_diagnostic *diagnostic;
	void *arg;
};

static bool __attribute__((format(printf, 2, 3))) rsc_map_warning(
	const struct rsc_map_diagnostic *map_diagnostic, char *fmt, ...)
{
	if (map_diagnostic->diagnostic) {
		va_list ap;

		va_start(ap, fmt);
		report_msg(map_diagnostic->diagnostic->warning,
			map_diagnostic->arg, "", "", fmt, ap);
		va_end(ap);
	}

	return false;
}

static bool __attribute__((format(printf, 2, 3))) rsc_map_error(
	const struct rsc_map_diagnostic *map_diagnostic, char *fmt, ...)
{
	if (map_diagnostic->diagnostic) {
		va_list ap;

		va_start(ap, fmt);
		report_msg(map_diagnostic->diagnostic->error,
			map_diagnostic->arg, "", "", fmt, ap);
		va_end(ap);
	}

	return false;
}

const char *rsc_map_entry_type_symbol(enum rsc_map_entry_type type)
{
	switch (type) {
#define RSC_MAP_ENTRY_TYPE_STRING(n_, symbol_)				\
	case rsc_map_entry_type_ ## symbol_: return #symbol_;
RSC_MAP_ENTRY_TYPE(RSC_MAP_ENTRY_TYPE_STRING)
	default: return "";
	}
}

static bool rsc_map_mark_type(const enum rsc_map_entry_type type,
	const size_t offset, const size_t size, struct rsc_map *map)
{
	if (offset + size > map->size)
		return false;

	for (size_t i = 0; i < size; i++)
		if (map->entry[offset + i].start ||
		    map->entry[offset + i].type)
			return false;

	if (size) {
		map->entry[offset].start = 1;
		map->entry[offset].type = type;
	}

	for (size_t i = 1; i < size; i++)
		map->entry[offset + i].type = type;

	return true;
}

static bool rsc_map_marked_type(const enum rsc_map_entry_type type,
	const size_t offset, const size_t size, struct rsc_map *map)
{
	if (offset + size > map->size)
		return false;

	for (size_t i = 0; i < size; i++)
		if (map->entry[offset + i].start != (i == 0) ||
		    map->entry[offset + i].type != type)
			return false;

	return true;
}

static bool rsc_map_marked_unused(
	const size_t offset, const size_t size, struct rsc_map *map)
{
	if (offset + size > map->size)
		return false;

	for (size_t i = 0; i < size; i++)
		if (map->entry[offset + i].type != rsc_map_entry_type_unused)
			return false;

	return true;
}

static bool rsc_map_mark_type_reuse(const enum rsc_map_entry_type type,
	const size_t offset, const size_t size, struct rsc_map *map)
{
	if (offset + size > map->size)
		return false;

	if (size &&
	    map->entry[offset].start &&
	    map->entry[offset].type == type)
		return true;

	return rsc_map_mark_type(type, offset, size, map);
}

static bool rsc_map_mark_type_reserved(const enum rsc_map_entry_type type,
	const size_t offset, const size_t size, struct rsc_map *map)
{
	if (offset + size > map->size)
		return false;

	for (size_t i = 0; i < size; i++)
		if (map->entry[offset + i].type != rsc_map_entry_type_unused &&
		    map->entry[offset + i].type != type)
			return false;
		else if (map->entry[offset + i].reserved)
			return false;

	for (size_t i = 0; i < size; i++)
		if (map->entry[offset + i].type != type) {
			map->entry[offset + i].reserved = 1;
			map->entry[offset + i].type = type;
		}

	return true;
}

static bool rsc_map_header(struct rsc_map_diagnostic *map_diagnostic)
{
	return rsc_map_mark_type(rsc_map_entry_type_header,
		0, sizeof(struct rsc_header), map_diagnostic->map);
}

static bool rsc_map_frstr(struct rsc_map_diagnostic *map_diagnostic)
{
	const struct rsc_header *h = map_diagnostic->rsc->header;

	if (!rsc_map_mark_type(rsc_map_entry_type_frstr, h->rsh_frstr,
			sizeof(uint32_t[h->rsh_nstring]), map_diagnostic->map))
		return false;

	for (size_t i = 0; i < h->rsh_nstring; i++) {
		const char *s = rsc_string_at_index(i, map_diagnostic->rsc);

		if (!s || !rsc_map_mark_type_reuse(rsc_map_entry_type_string,
				rsc_string_offset_at_index(i, map_diagnostic->rsc),
				strlen(s) + 1, map_diagnostic->map))
			return false;
	}

	return true;
}

static bool rsc_map_bitblk(const size_t bitblk_offset,
	struct rsc_map *map, const struct rsc *rsc)
{
	const struct rsc_bitblk *bitblk =
		rsc_bitblk_at_offset(bitblk_offset, rsc);

	if (!bitblk)
		return false;

	if (!rsc_map_mark_type(rsc_map_entry_type_bitblk,
			bitblk_offset, sizeof(struct rsc_bitblk), map))
		return false;

	if (bitblk->bi_wb % 2)
		return false;

	if (!rsc_map_mark_type(rsc_map_entry_type_imdata,
			bitblk->bi_data, bitblk->bi_wb * bitblk->bi_hl, map))
		return false;

	return true;
}

static bool rsc_map_frimg(struct rsc_map_diagnostic *map_diagnostic)
{
	const struct rsc_header *h = map_diagnostic->rsc->header;

	if (!rsc_map_mark_type(rsc_map_entry_type_frimg, h->rsh_frimg,
			sizeof(uint32_t[h->rsh_nimages]), map_diagnostic->map))
		return false;

	for (size_t i = 0; i < h->rsh_nimages; i++)
		if (!rsc_map_bitblk(rsc_frimg_offset_at_index(
				i, map_diagnostic->rsc),
				map_diagnostic->map, map_diagnostic->rsc))
			return false;

	return true;
}

static bool rsc_valid_object_index(const int16_t ob,
	const size_t tree_offset, const struct rsc *rsc)
{
	if (ob < 0 && ob != -1)
		return false;

	return tree_offset + sizeof(struct rsc_object[ob + 1]) <=
		rsc_unextended_size(rsc);
}

static bool rsc_map_string(const size_t string_offset,
	struct rsc_map *map, const struct rsc *rsc)
{
	const char *s = rsc_string_at_offset(string_offset, rsc);

	if (!s)
		return false;

	if (!rsc_map_mark_type_reuse(rsc_map_entry_type_string,
			string_offset, strlen(s) + 1, map))
		return false;

	return true;
}

static bool rsc_map_tedinfo(const size_t tedinfo_offset,
	struct rsc_map *map, const struct rsc *rsc)
{
	const struct rsc_tedinfo *tedinfo =
		rsc_tedinfo_at_offset(tedinfo_offset, rsc);

	if (!tedinfo)
		return false;

	if (!rsc_map_mark_type(rsc_map_entry_type_tedinfo,
			tedinfo_offset, sizeof(struct rsc_tedinfo), map))
		return false;

	return rsc_map_string(tedinfo->te_text,  map, rsc) &&
	       rsc_map_string(tedinfo->te_tmplt, map, rsc) &&
	       rsc_map_string(tedinfo->te_valid, map, rsc);
}

static bool rsc_map_iconblk(const size_t iconblk_offset,
	struct rsc_map *map, const struct rsc *rsc)
{
	const struct rsc_iconblk *iconblk =
		rsc_iconblk_at_offset(iconblk_offset, rsc);

	if (!iconblk)
		return false;

	if (!rsc_map_mark_type(rsc_map_entry_type_iconblk,
			iconblk_offset, sizeof(struct rsc_iconblk), map))
		return false;

	if (iconblk->ib_icon.r.w % 16)
		return false;

	if (!rsc_map_mark_type(rsc_map_entry_type_imdata, iconblk->ib_mask,
			(iconblk->ib_icon.r.w / 8) * iconblk->ib_icon.r.h, map))
		return false;

	if (!rsc_map_mark_type(rsc_map_entry_type_imdata, iconblk->ib_data,
			(iconblk->ib_icon.r.w / 8) * iconblk->ib_icon.r.h, map))
		return false;

	if (!rsc_map_string(iconblk->ib_text, map, rsc))
		return false;

	return true;
}

static bool rsc_map_applblk(const size_t applblk_offset,
	struct rsc_map *map, const struct rsc *rsc)
{
	return rsc_map_mark_type(rsc_map_entry_type_applblk,
		applblk_offset, sizeof(struct rsc_applblk), map);

}

static bool rsc_map_object(const int16_t ob, const struct rsc_object *tree,
	const size_t tree_offset, struct rsc_map_diagnostic *map_diagnostic)
{
	struct rsc_map *map = map_diagnostic->map;
	const struct rsc *rsc = map_diagnostic->rsc;

	if (!rsc_valid_object_index(tree[ob].link.next, tree_offset, rsc) ||
	    !rsc_valid_object_index(tree[ob].link.head, tree_offset, rsc) ||
	    !rsc_valid_object_index(tree[ob].link.tail, tree_offset, rsc))
		return false;

	if (!rsc_map_mark_type(rsc_map_entry_type_object,
			tree_offset + sizeof(struct rsc_object[ob]),
			sizeof(*tree), map))
		return false;

	switch (tree[ob].shape.type.g) {
	case GEM_G_BOX:
	case GEM_G_IBOX:
	case GEM_G_BOXCHAR:
		break;
	case GEM_G_TEXT:
	case GEM_G_BOXTEXT:
	case GEM_G_FTEXT:
	case GEM_G_FBOXTEXT:
		if (!rsc_map_tedinfo(tree[ob].shape.spec.tedinfo, map, rsc))
			return false;
		break;
	case GEM_G_IMAGE:
		if (!rsc_map_bitblk(tree[ob].shape.spec.bitblk, map, rsc))
			return false;
		break;
	case GEM_G_BUTTON:
	case GEM_G_STRING:
	case GEM_G_TITLE:
		if (!rsc_map_string(tree[ob].shape.spec.string, map, rsc))
			return false;
		break;
	case GEM_G_ICON:
		if (!rsc_map_iconblk(tree[ob].shape.spec.iconblk, map, rsc))
			return false;
		break;
	case GEM_G_PROGDEF:
		if (!rsc_map_applblk(tree[ob].shape.spec.applblk, map, rsc))
			return false;
		break;
	default:
		rsc_map_warning(map_diagnostic,
			"Object %d undefined type %d", ob, tree[ob].shape.type.g);
	}

	return true;
}

static bool rsc_mapped_object(const int16_t ob, const struct rsc_object *tree,
	const size_t tree_offset, struct rsc_map *map)
{
	if (ob < 0)
		return false;

	return rsc_map_marked_type(rsc_map_entry_type_object,
			tree_offset + sizeof(struct rsc_object[ob]),
			sizeof(*tree), map);
}

static bool rsc_map_tree_objects(const struct rsc_object *tree,
	const size_t i, struct rsc_map_diagnostic *map_diagnostic)
{
	const size_t tree_offset =
		rsc_tree_offset_at_index(i, map_diagnostic->rsc);
	int16_t lastob = 0;

	for (int16_t ob = 0; rsc_valid_ob(ob); ob = rsc_tree_traverse(ob, tree))
		if (!rsc_map_object(ob, tree, tree_offset, map_diagnostic))
			return rsc_map_error(map_diagnostic,
				"Tree %zu object %d malformed", i, ob);

	for (int16_t ob = 0; rsc_valid_ob(ob); ob = rsc_tree_traverse(ob, tree)) {
		if (tree[ob].link.head == -1)
			continue;

		if (tree[ob].link.tail < 0)
			return rsc_map_error(map_diagnostic,
				"Tree %zu object %d invalid tail %d",
				i, ob, tree[ob].link.tail);

		if (!rsc_mapped_object(tree[ob].link.tail,
				tree, tree_offset, map_diagnostic->map))
			return rsc_map_error(map_diagnostic,
				"Tree %zu object %d malformed", i, ob);

		if (tree[tree[ob].link.tail].link.next != ob) /* Parent link */
			return rsc_map_error(map_diagnostic,
				"Tree %zu object %d invalid parent %d != %d",
				i, ob, tree[tree[ob].link.tail].link.next, ob);
	}

	for (int16_t ob = 0; rsc_valid_ob(ob); ob = rsc_tree_traverse(ob, tree)) {
		if (tree[ob].shape.flags.lastob &&
		    rsc_valid_ob(rsc_tree_traverse(ob, tree)))
			return rsc_map_error(map_diagnostic,
				"Tree %zu object %d not last object", i, ob);

		lastob = ob;
	}

	if (!tree[lastob].shape.flags.lastob)
		return rsc_map_error(map_diagnostic,
			"Tree %zu object %d lastob unset", i, lastob);

	return true;
}

static bool rsc_map_tree(struct rsc_map_diagnostic *map_diagnostic)
{
	const struct rsc_header *h = map_diagnostic->rsc->header;

	for (size_t i = 0; i < h->rsh_ntree; i++) {
		const struct rsc_object *tree =
			rsc_tree_at_index(i, map_diagnostic->rsc);

		if (!tree)
			return rsc_map_error(map_diagnostic, "Tree %zu not found", i);

		if (!rsc_map_mark_type(rsc_map_entry_type_trindex,
				h->rsh_trindex + sizeof(uint32_t[i]),
				sizeof(uint32_t), map_diagnostic->map))
			return rsc_map_error(map_diagnostic, "Tree %zu malformed", i);

		if (!rsc_map_tree_objects(tree, i, map_diagnostic))
			return rsc_map_error(map_diagnostic, "Tree %zu malformed objects", i);
	}

	return true;
}

static bool rsc_map_padding(struct rsc_map_diagnostic *map_diagnostic)
{
	enum rsc_map_entry_type entry_type = rsc_map_entry_type_unused;
	struct rsc_map_region region;

	rsc_map_for_each_region (region, map_diagnostic->map) {
		if (entry_type == rsc_map_entry_type_string &&
		    region.entry.type == rsc_map_entry_type_unused &&
		    region.offset % 2 &&
		    region.size == 1)
			if (!rsc_map_mark_type(rsc_map_entry_type_padding,
					region.offset, region.size,
					map_diagnostic->map))
				return false;

		entry_type = region.entry.type;
	}

	return true;
}

static bool rsc_map_alignment(struct rsc_map_diagnostic *map_diagnostic)
{
	struct rsc_map_region region;

	rsc_map_for_each_region (region, map_diagnostic->map)
		if (region.entry.type == rsc_map_entry_type_unused ||
		    region.entry.type == rsc_map_entry_type_string ||
		    region.entry.type == rsc_map_entry_type_padding)
			continue;
		else if (region.offset % 2 != 0)
			return rsc_map_error(map_diagnostic,
				"Unaligned %s at offset %zu",
				rsc_map_entry_type_symbol(region.entry.type),
				region.offset);

	return true;
}

static bool rsc_map_reservations(struct rsc_map_diagnostic *map_diagnostic)
{
	const struct rsc_header *h = map_diagnostic->rsc->header;

	const struct {
		enum rsc_map_entry_type type;
		size_t offset;
		size_t size;
	} r[] = {
		{ rsc_map_entry_type_object,  h->rsh_object,  sizeof(struct rsc_object[h->rsh_nobs])  },
		{ rsc_map_entry_type_tedinfo, h->rsh_tedinfo, sizeof(struct rsc_tedinfo[h->rsh_nted]) },
		{ rsc_map_entry_type_iconblk, h->rsh_iconblk, sizeof(struct rsc_iconblk[h->rsh_nib])  },
		{ rsc_map_entry_type_bitblk,  h->rsh_bitblk,  sizeof(struct rsc_bitblk[h->rsh_nbb])   },
	};

	for (size_t i = 0; i < ARRAY_SIZE(r); i++)
		if (!rsc_map_mark_type_reserved(r[i].type,
				r[i].offset, r[i].size, map_diagnostic->map))
			return false;

	return true;
}

static bool rsc_map_unextended(struct rsc_map_diagnostic *map_diagnostic)
{
	const size_t unextended_size = rsc_unextended_size(map_diagnostic->rsc);

	if (map_diagnostic->map->size < unextended_size)
		return false;

	return rsc_map_marked_unused(unextended_size,
		map_diagnostic->map->size - unextended_size,
		map_diagnostic->map);
}

static size_t rsc_map_region_size(const size_t offset, struct rsc_map *map)
{
	size_t i;

	for (i = 0; offset + i < map->size; i++)
		if (map->entry[offset + i].type != map->entry[offset].type ||
		    map->entry[offset + i].reserved != map->entry[offset].reserved ||
		    (i > 0 && map->entry[offset + i].start))
			break;

	return i;
}

struct rsc_map_region rsc_map_first_region(struct rsc_map *map)
{
	const size_t size = rsc_map_region_size(0, map);

	return size ? (struct rsc_map_region) {
		.entry = map->entry[0],
		.offset = 0,
		.size = size,
	} : (struct rsc_map_region) { };
}

struct rsc_map_region rsc_map_next_region(
	struct rsc_map_region region, struct rsc_map *map)
{
	const size_t offset = region.offset + region.size;
	const size_t size = rsc_map_region_size(offset, map);

	return size ? (struct rsc_map_region) {
		.entry = map->entry[offset],
		.offset = offset,
		.size = size,
	} : (struct rsc_map_region) { };
}

struct rsc_map *rsc_map_alloc(const struct rsc *rsc)
{
	const size_t size =
		sizeof(struct rsc_map) + sizeof(struct rsc_map_entry[rsc->size]);
	struct rsc_map *map = malloc(size);

	if (map) {
		memset(map, 0, size);

		map->size = rsc->size;
		map->entry = (struct rsc_map_entry *)&map[1];
	}

	return map;
}

void rsc_map_free(struct rsc_map *map)
{
	free(map);
}

static bool rsc_map_diagnostic(struct rsc_map_diagnostic *map_diagnostic)
{
	BUILD_BUG_ON(sizeof(struct rsc_map_entry) != 1);

	memset(map_diagnostic->map->entry, 0, map_diagnostic->map->size);

	if (!rsc_map_header(map_diagnostic))
		return rsc_map_error(map_diagnostic, "Malformed header");

	if (!rsc_map_frstr(map_diagnostic))
		return rsc_map_error(map_diagnostic, "Malformed frstr");

	if (!rsc_map_frimg(map_diagnostic))
		return rsc_map_error(map_diagnostic, "Malformed frimg");

	if (!rsc_map_tree(map_diagnostic))
		return rsc_map_error(map_diagnostic, "Malformed tree");

	if (!rsc_map_padding(map_diagnostic))
		return rsc_map_error(map_diagnostic, "Malformed padding");

	if (!rsc_map_alignment(map_diagnostic))
		return false;

	if (!rsc_map_reservations(map_diagnostic))
		return rsc_map_error(map_diagnostic, "Malformed reservations");

	if (!rsc_map_unextended(map_diagnostic))
		return rsc_map_error(map_diagnostic, "Malformed unextended structure");

	return true;
}

bool rsc_map(struct rsc_map *map, const struct rsc *rsc)
{
	struct rsc_map_diagnostic map_diagnostic = { .map = map, .rsc = rsc };

	return rsc_map_diagnostic(&map_diagnostic);
}

bool rsc_valid_map(const struct rsc *rsc,
	const struct rsc_diagnostic *diagnostic, void *arg)
{
	struct rsc_map_diagnostic map_diagnostic = {
		.map = rsc_map_alloc(rsc),
		.rsc = rsc,

		.diagnostic = diagnostic,
		.arg = arg
	};

	if (!map_diagnostic.map)
		return rsc_map_error(&map_diagnostic,
			"Memory allocation failed");

	const bool valid = rsc_map_diagnostic(&map_diagnostic);

	rsc_map_free(map_diagnostic.map);

	return valid;
}
