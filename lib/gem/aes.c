// SPDX-License-Identifier: GPL-2.0

#include <stdlib.h>
#include <string.h>

#include <gem/aes.h>
#include <gem/aes-area.h>
#include <gem/aes-filter.h>
#include <gem/aes-pixel.h>
#include <gem/aes-shape.h>
#include <gem/aes-simple.h>
#include <gem/vdi_.h>

#include "internal/assert.h"

aes_id_t aes_appl_init(struct aes *aes_)
{
	vdi_id_t vdi_id = vdi_v_opnwk(NULL, NULL);

	*aes_ = (struct aes) { .vdi_id = vdi_id };

	return (aes_id_t) { .aes_ = aes_ };
}

void aes_appl_exit(aes_id_t aes_id)
{
	vdi_v_clswk(aes_id.aes_->vdi_id);
}

bool aes_palette_color(aes_id_t aes_id,
	const int index, struct vdi_color *color)
{
	return vq_color(aes_id.aes_->vdi_id, index, color);
}
