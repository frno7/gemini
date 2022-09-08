// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#include <gem/aes-area.h>
#include <gem/aes-shape.h>

struct aes_object_border {
	int color;
	int thickness;
};

static struct aes_object_border aes_object_shape_border(
	const struct aes_object_shape *shape)
{
	switch (shape->type.g) {
	case GEM_G_BOX:
	case GEM_G_BOXCHAR:
		return (struct aes_object_border) {
			.color = shape->spec.box.color.border,
			.thickness = shape->spec.box.thickness
		};
	case GEM_G_BUTTON:
		return (struct aes_object_border) {
			.color = 1,
			.thickness = (shape->flags.exit     ? -1 : 0) +
				     (shape->flags.default_ ? -1 : 0) - 1
		};
	}

	return (struct aes_object_border) { };
}

static struct aes_object_simple_shape_enumerator aes_object_simple_shape(
	const int i, struct aes_object_shape *simple,
	struct aes_object_shape shape)
{
	const struct aes_object_border border = aes_object_shape_border(&shape);
	int n = 0;

	if (shape.state.outlined) {
		if (i == n++) {
			*simple = (struct aes_object_shape) {
				.type = { .g = GEM_G_BOX },
				.spec = {
					.box = {
						.color = {
							.fill = 1,
							.pattern = 7
						}
					}
				},
				.area = aes_area_shrink(shape.area, -3)
			};
		}

		if (i == n++) {
			*simple = (struct aes_object_shape) {
				.type = { .g = GEM_G_BOX },
				.spec = {
					.box = {
						.color = {
							.fill = 0,
							.pattern = 7
						}
					}
				},
				.area = aes_area_shrink(shape.area, -2)
			};
		}
	}

	if (border.thickness) {
		if (i == n++) {
			*simple = (struct aes_object_shape) {
				.type = { .g = GEM_G_BOX },
				.spec = {
					.box = {
						.color = {
							.fill = border.color,
							.pattern = 7
						}
					}
				},
				.area = border.thickness > 0 ? shape.area :
					aes_area_shrink(shape.area, border.thickness)
			};
		}
	}

	if (i == n++) {
		if (border.thickness > 0)
			shape.area = aes_area_shrink(shape.area, border.thickness);

		*simple = shape;
	}

	return (struct aes_object_simple_shape_enumerator) { .i = i, .n = n };
}

struct aes_object_simple_shape_enumerator aes_object_first_simple_shape(
	struct aes_object_shape *simple, const struct aes_object_shape shape)
{
	return aes_object_simple_shape(0, simple, shape);
}

struct aes_object_simple_shape_enumerator aes_object_next_simple_shape(
	struct aes_object_shape *simple, const struct aes_object_shape shape,
	struct aes_object_simple_shape_enumerator e)
{
	return aes_object_simple_shape(e.i + 1, simple, shape);
}

static bool aes_object_first_simple_shape_(
	struct aes_object_shape *shape,
	struct aes_object_shape_iterator *iterator)
{
	struct aes_object_simple_shape_iterator_arg *arg = iterator->arg;

	if (!arg->iterator.first(&arg->shape, &arg->iterator))
		return false;

	arg->enumerator = aes_object_first_simple_shape(shape, arg->shape);

	return arg->enumerator.i < arg->enumerator.n;
}

static bool aes_object_next_simple_shape_(
	struct aes_object_shape *shape,
	struct aes_object_shape_iterator *iterator)
{
	struct aes_object_simple_shape_iterator_arg *arg = iterator->arg;

	if (arg->enumerator.i < arg->enumerator.n)
		arg->enumerator = aes_object_next_simple_shape(
			shape, arg->shape, arg->enumerator);

	if (arg->enumerator.i >= arg->enumerator.n) {
		if (!arg->iterator.next(&arg->shape, &arg->iterator))
			return false;

		arg->enumerator = aes_object_first_simple_shape(
			shape, arg->shape);
	}

	return arg->enumerator.i < arg->enumerator.n;
}

struct aes_object_shape_iterator aes_object_simple_shape_iterator(
	struct aes_object_shape_iterator *iterator,
	struct aes_object_simple_shape_iterator_arg *arg)
{
	*arg = (struct aes_object_simple_shape_iterator_arg) {
		.iterator = *iterator
	};

	return (struct aes_object_shape_iterator) {
		.first = aes_object_first_simple_shape_,
		.next  = aes_object_next_simple_shape_,
		.arg   = arg
	};
}
