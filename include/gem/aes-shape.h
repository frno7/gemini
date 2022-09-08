// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#ifndef _GEM_AES_SHAPE_H
#define _GEM_AES_SHAPE_H

#include "aes.h"

struct aes_object_simple_shape_enumerator {
	int i;
	int n;
};

#define aes_for_each_object_shape(shape, iterator)			\
	for (bool valid__ = (iterator)->first((shape), (iterator));	\
	     valid__;							\
	     valid__ = (iterator)->next((shape), (iterator)))

#define aes_for_each_simple_object_shape(simple, shape)			\
	for (struct aes_object_simple_shape_enumerator e__ =		\
			aes_object_first_simple_shape((simple), (shape));\
	     e__.i < e__.n;					\
	     e__ = aes_object_next_simple_shape((simple), (shape), e__))

struct aes_object_simple_shape_enumerator aes_object_first_simple_shape(
	struct aes_object_shape *simple, const struct aes_object_shape shape);

struct aes_object_simple_shape_enumerator aes_object_next_simple_shape(
	struct aes_object_shape *simple, const struct aes_object_shape shape,
	struct aes_object_simple_shape_enumerator e);

struct aes_object_simple_shape_iterator_arg {
	struct aes_object_shape shape;
	struct aes_object_shape_iterator iterator;
	struct aes_object_simple_shape_enumerator enumerator;
};

struct aes_object_shape_iterator aes_object_simple_shape_iterator(
	struct aes_object_shape_iterator *iterator,
	struct aes_object_simple_shape_iterator_arg *arg);

#endif /* _GEM_AES_SHAPE_H */
