// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#include <gem/aes-area.h>
#include <gem/aes-shape.h>
#include <gem/aes-filter.h>

static bool aes_object_first_filter_shape(
	struct aes_object_shape *shape,
	struct aes_object_shape_iterator *iterator)
{
	struct aes_object_filter_shape_iterator_arg *arg = iterator->arg;

	if (!arg->iterator.first(shape, &arg->iterator))
		return false;

	while (!arg->f(shape, arg->f_arg))
		if (!arg->iterator.next(shape, &arg->iterator))
			return false;

	return true;
}

static bool aes_object_next_filter_shape(
	struct aes_object_shape *shape,
	struct aes_object_shape_iterator *iterator)
{
	struct aes_object_filter_shape_iterator_arg *arg = iterator->arg;

	if (!arg->iterator.next(shape, &arg->iterator))
		return false;

	while (!arg->f(shape, arg->f_arg))
		if (!arg->iterator.next(shape, &arg->iterator))
			return false;

	return true;
}

struct aes_object_shape_iterator aes_object_filter_shape_iterator(
	aes_object_filter_shape_f f, void *f_arg,
	struct aes_object_shape_iterator *iterator,
	struct aes_object_filter_shape_iterator_arg *arg)
{
	*arg = (struct aes_object_filter_shape_iterator_arg) {
		.iterator = *iterator,
		.f = f,
		.f_arg = f_arg
	};

	return (struct aes_object_shape_iterator) {
		.first = aes_object_first_filter_shape,
		.next  = aes_object_next_filter_shape,
		.arg   = arg
	};
}
