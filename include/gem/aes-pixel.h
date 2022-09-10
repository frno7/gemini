// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#ifndef _GEM_AES_PIXEL_H
#define _GEM_AES_PIXEL_H

#include "aes.h"

int aes_object_shape_pixel(aes_id_t aes_id, const struct aes_point p,
	const struct aes_object_shape *shape);

#endif /* _GEM_AES_PIXEL_H */
