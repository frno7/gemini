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

#endif /* _GEM_AES_RSC_H */