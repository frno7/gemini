// SPDX-License-Identifier: GPL-2.0

#include <stdlib.h>
#include <string.h>

#include <gem/color.h>
#include <gem/vdi_.h>

#include "internal/print.h"
#include "internal/storage.h"
#include "internal/string.h"

void vdi_v_clswk(vdi_id_t vdi_id)
{
	struct vdi_fnt *vdi_fnt;

	while ((vdi_fnt = list_first_entry_or_null(
			&vdi_id.vdi->font.list, struct vdi_fnt, list))) {
		list_del(&vdi_fnt->list);
		free(vdi_fnt);
	}

	free(vdi_id.vdi);
}

vdi_id_t vdi_v_opnwk(const struct vdi_workstation_default *wsd,
	struct vdi_workstation *ws)
{
	struct vdi_ *vdi = malloc(sizeof(struct vdi_));

	if (!vdi)
		return (vdi_id_t) { };

	*vdi = (struct vdi_) {
		.palette = {
			.count = 16,
			.colors = {
#define VDI_SYSTEM_COLOR(n_, r_, g_, b_, symbol_, label_, description_)\
				[n_] = { .r = r_, .g = g_, .b = b_ },
GEM_PALETTE_COLOR(VDI_SYSTEM_COLOR)
			}
		}
	};

	vdi_id_t vdi_id = (vdi_id_t) { .vdi = vdi };

	const struct storage_file *file;

	INIT_LIST_HEAD(&vdi_id.vdi->font.list);
        storage_for_each_file (file)
		if (strsuffix(".fnt", file->path)) {
			const struct fnt fnt = {
				.size = file->size,
				.header = (struct fnt_header *)file->data
			};

			if (!vdi_v_fontinit(vdi_id, &fnt))
				pr_warn("%s: vdi_v_fontinit failed\n",
					file->path);
		}

	return vdi_id;
}

bool vq_color(const vdi_id_t vdi_id, const int index, struct vdi_color *color)
{
	if (index < 0 && index >= vdi_id.vdi->palette.count)
		return false;

	*color = vdi_id.vdi->palette.colors[index];

	return true;
}

bool vdi_v_fontinit(vdi_id_t vdi_id, const struct fnt *fnt)
{
	if (!fnt_valid(fnt))
		return false;

	struct vdi_fnt *vdi_fnt =
		malloc(sizeof(*vdi_fnt) + fnt->size);

	if (!vdi_fnt)
		return false;

	void *d = &vdi_fnt[1];

	*vdi_fnt = (struct vdi_fnt) {
		.fnt = {
			.size = fnt->size,
			.header = d
		}
	};
	memcpy(d, fnt->header, fnt->size);

	list_add(&vdi_fnt->list, &vdi_id.vdi->font.list);

	if (!vdi_id.vdi->font.large ||
	    fnt_cmp(vdi_id.vdi->font.large, &vdi_fnt->fnt) <= 0)
		vdi_id.vdi->font.large = &vdi_fnt->fnt;

	if (!vdi_id.vdi->font.small ||
	    fnt_cmp(vdi_id.vdi->font.small, &vdi_fnt->fnt) >= 0)
		vdi_id.vdi->font.small = &vdi_fnt->fnt;

	return true;
}
