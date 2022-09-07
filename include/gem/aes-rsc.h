// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#ifndef _GEM_AES_RSC_H
#define _GEM_AES_RSC_H

#include "aes.h"
#include "rsc.h"

struct aes_object_shape aes_rsc_object_shape(aes_id_t aes_id,
	const struct aes_point p, const struct rsc_object *ro,
	const struct rsc *rsc);

int16_t aes_rsc_tree_traverse_with_origin(aes_id_t aes_id,
	struct aes_point *origin, int16_t ob, const struct rsc_object *tree);

struct aes_area aes_rsc_tree_bounds(aes_id_t aes_id,
	const struct rsc_object *tree, const struct rsc *rsc);

struct aes_rsc_object_shape_iterator_arg {
	struct aes_point origin;
	aes_id_t aes_id;
	int16_t ob;
	const struct rsc_object *tree;
	const struct rsc *rsc;
};

struct aes_object_shape_iterator aes_rsc_object_shape_iterator(
	aes_id_t aes_id, const struct rsc_object *tree, const struct rsc *rsc,
	struct aes_rsc_object_shape_iterator_arg *arg);

#endif /* _GEM_AES_RSC_H */
