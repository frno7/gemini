// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#ifndef _GEM_AES_FILTER_H
#define _GEM_AES_FILTER_H

#include "aes.h"

typedef bool (*aes_object_filter_shape_f)(
	struct aes_object_shape *shape, void *arg);

struct aes_object_filter_shape_iterator_arg {
	struct aes_object_shape_iterator iterator;
	aes_object_filter_shape_f f;
	void *f_arg;
};

struct aes_object_shape_iterator aes_object_filter_shape_iterator(
	aes_object_filter_shape_f f, void *f_arg,
	struct aes_object_shape_iterator *iterator,
	struct aes_object_filter_shape_iterator_arg *arg);

#endif /* _GEM_AES_FILTER_H */
