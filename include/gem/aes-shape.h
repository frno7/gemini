// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#ifndef _GEM_AES_SHAPE_H
#define _GEM_AES_SHAPE_H

#include "aes.h"

struct aes_area aes_object_shape_bounds(
	struct aes_object_shape_iterator *iterator);

#endif /* _GEM_AES_SHAPE_H */
