// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#include <gem/aes-area.h>
#include <gem/aes-shape.h>
#include <gem/aes-simple.h>

struct aes_area aes_object_shape_bounds(
	struct aes_object_shape_iterator *iterator)
{
	struct aes_object_simple_shape_iterator_arg simple_arg;
	struct aes_object_shape_iterator i =
		aes_object_simple_shape_iterator(iterator, &simple_arg);
	struct aes_object_shape shape;
	struct aes_area bounds = { };
	int k = 0;

	aes_for_each_object_shape (&shape, &i)
		bounds = !k++ ? shape.area : aes_area_bounds(bounds, shape.area);

	return bounds;
}
