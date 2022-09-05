// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#ifndef _GEM_AES_AREA_H
#define _GEM_AES_AREA_H

#include "aes.h"

#include "internal/compare.h"

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

#endif /* _GEM_AES_AREA_H */
