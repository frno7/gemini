// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2021 Fredrik Noring
 */

#ifndef _GEM_AES_H
#define _GEM_AES_H

#include <stdbool.h>
#include <stdint.h>

#include "rsc.h"
#include "vdi.h"

struct aes_point {
	int x;
	int y;
};

struct aes_rectangle {
	int w;
	int h;
};

struct aes_area {
	struct aes_point p;
	struct aes_rectangle r;
};

struct aes {
	vdi_id_t vdi_id;
};

typedef struct {
	struct aes *aes_;
} aes_id_t;

aes_id_t aes_appl_init(struct aes *aes_);

static inline bool aes_id_valid(aes_id_t aes_id)
{
	return vdi_id_valid(aes_id.aes_->vdi_id);
}

struct fnt *aes_fnt_large(aes_id_t aes_id);

struct fnt *aes_fnt_small(aes_id_t aes_id);

void aes_appl_exit(aes_id_t aes_id);

bool aes_palette_color(aes_id_t aes_id,
	const int index, struct vdi_color *color);

struct aes_area aes_objc_bounds(aes_id_t aes_id,
	const int ob, const struct rsc_object *tree, const struct rsc *rsc_);

int aes_objc_pixel(aes_id_t aes_id, const struct aes_point p,
	const struct rsc_object *tree, const struct rsc *rsc_);

#endif /* _GEM_AES_H */
