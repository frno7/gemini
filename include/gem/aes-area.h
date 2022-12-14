// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#ifndef _GEM_AES_AREA_H
#define _GEM_AES_AREA_H

#include "aes.h"

#include "internal/compare.h"

static inline bool aes_point_within_area(const struct aes_point p,
	const struct aes_area area)
{
	return p.x >= area.p.x &&
	       p.y >= area.p.y &&
	       p.x <  area.p.x + area.r.w &&
	       p.y <  area.p.y + area.r.h;
}

typedef struct aes_area (*aes_area_justify_rectangle_f)(
	const struct aes_rectangle rectangle,
	const struct aes_area area);

static inline struct aes_area aes_area_justify_rectangle_top_center(
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

static inline struct aes_area aes_area_justify_rectangle_center_left(
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

static inline struct aes_area aes_area_justify_rectangle_center(
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

static inline struct aes_area aes_area_justify_rectangle_center_right(
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

static inline struct aes_area aes_area_shrink(
	const struct aes_area area, const int amount)
{
	return aes_area_justify_rectangle_center(
		(struct aes_rectangle) {
			.w = max(0, area.r.w - 2 * amount),
			.h = max(0, area.r.h - 2 * amount)
		}, area);
}

static inline struct aes_area aes_area_bounds(
	struct aes_area a,
	struct aes_area b)
{
	const struct aes_point pmin = {
		.x = min(a.p.x, b.p.x),
		.y = min(a.p.y, b.p.y)
	};
	const struct aes_point pmax = {
		.x = max(a.p.x + a.r.w, b.p.x + b.r.w),
		.y = max(a.p.y + a.r.h, b.p.y + b.r.h)
	};

	return (struct aes_area) {
		.p = pmin,
		.r = {
			.w = pmax.x - pmin.x,
			.h = pmax.y - pmin.y
		},
	};
}

static inline struct aes_area aes_area_intersection(
	struct aes_area a,
	struct aes_area b)
{
	const struct aes_point tlp = (struct aes_point) {
		.x = max(a.p.x, b.p.x),
		.y = max(a.p.y, b.p.y)
	};
	const struct aes_point brp = (struct aes_point) {
		.x = min(a.p.x + a.r.w, b.p.x + b.r.w),
		.y = min(a.p.y + a.r.h, b.p.y + b.r.h)
	};

	return tlp.x <= brp.x && tlp.y <= brp.y ?
		(struct aes_area) {
			.p = tlp,
			.r = {
				.w = brp.x - tlp.x,
				.h = brp.y - tlp.y
			},
		} : (struct aes_area) { };
}

static inline bool aes_area_degenerate(struct aes_area area)
{
	return area.r.w <= 0 ||
	       area.r.h <= 0;
}

static inline bool aes_area_overlap(
	struct aes_area a,
	struct aes_area b)
{
	return !aes_area_degenerate(aes_area_intersection(a, b));
}

struct aes_area_subdivision_enumerator {
	int i;
	struct aes_area s[4];
};

struct aes_area_subdivision_enumerator aes_area_subdivide(
	const struct aes_area a,
	const struct aes_area b);

#define aes_for_each_area_subdivision(s_, a_, b_)			\
	for (struct aes_area_subdivision_enumerator subdivision__ =	\
		aes_area_subdivide((a_), (b_));				\
	     ({								\
	        const bool valid__ =					\
			subdivision__.i < ARRAY_SIZE(subdivision__.s);	\
		if (valid__)						\
			*(s_) = subdivision__.s[subdivision__.i];	\
		valid__;						\
	     });							\
	     subdivision__.i++)						\
		if (aes_area_degenerate(subdivision__.s[subdivision__.i]))\
			continue;					\
		else

#endif /* _GEM_AES_AREA_H */
