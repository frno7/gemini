// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#include <gem/aes-area.h>

struct aes_area_subdivision_enumerator aes_area_subdivide(
	const struct aes_area a,
	const struct aes_area b)
{
	const struct aes_area bounds = aes_area_bounds(a, b);

	return (struct aes_area_subdivision_enumerator) { .s = {
		aes_area_intersection(a, (struct aes_area)
		{
			.p = bounds.p,
			.r = {
				.w = bounds.r.w,
				.h = b.p.y - bounds.p.y
			},
		}),
		aes_area_intersection(a, (struct aes_area)
		{
			.p = {
				.x = bounds.p.x,
				.y = b.p.y
			},
			.r = {
				.w = b.p.x - bounds.p.x,
				.h = b.r.h
			},
		}),
		aes_area_intersection(a, (struct aes_area)
		{
			.p = {
				.x = b.p.x + b.r.w,
				.y = b.p.y
			},
			.r = {
				.w = (bounds.p.x + bounds.r.w) - (b.p.x + b.r.w),
				.h = b.r.h
			},
		}),
		aes_area_intersection(a, (struct aes_area)
		{
			.p = {
				.x = bounds.p.x,
				.y = b.p.y + b.r.h
			},
			.r = {
				.w = bounds.r.w,
				.h = (bounds.p.y + bounds.r.h) - (b.p.y + b.r.h)
			},
		})
	} };
}
