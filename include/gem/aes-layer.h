// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#ifndef _GEM_AES_LAYER_H
#define _GEM_AES_LAYER_H

#include "aes.h"

struct aes_object_shape_layer {
	struct aes_object_shape_layer *next;
	struct aes_object_shape shape;
};

typedef bool (*aes_object_shape_layer_f)(const struct aes_area clip,
	const struct aes_object_shape_layer *layers, void *arg);

bool aes_object_shape_layers(struct aes_area clip,
	struct aes_object_shape_iterator *iterator,
	const aes_object_shape_layer_f f, void *arg);

#endif /* _GEM_AES_LAYER_H */
