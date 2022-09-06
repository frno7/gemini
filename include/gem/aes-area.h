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

#endif /* _GEM_AES_AREA_H */
