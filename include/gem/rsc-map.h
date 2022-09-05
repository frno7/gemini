// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#ifndef _GEM_RSC_MAP_H
#define _GEM_RSC_MAP_H

#include <stdbool.h>
#include <stdint.h>

#include "rsc.h"

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

const char *rsc_map_entry_type_symbol(enum rsc_map_entry_type type);

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

bool rsc_valid_map(const struct rsc *rsc,
	const struct rsc_diagnostic *diagnostic, void *arg);

#endif /* _GEM_RSC_MAP_H */
