// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2021 Fredrik Noring
 */

#ifndef _GEM_VDI__H
#define _GEM_VDI__H

#include <stdbool.h>
#include <stdint.h>

#include "fnt.h"
#include "vdi.h"

#include "internal/list.h"

struct vdi_fnt {
	struct fnt fnt;
	struct list_head list;
};

struct vdi_ {
	struct vdi_palette {
		size_t count;
		struct vdi_color colors[256];
	} palette;

	struct {
		struct list_head list;
		struct fnt *large;
		struct fnt *small;
	} font;
};

#endif /* _GEM_VDI__H */
