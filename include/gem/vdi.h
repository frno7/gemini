// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2021 Fredrik Noring
 */

#ifndef _GEM_VDI_H
#define _GEM_VDI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "fnt.h"

struct vdi_;

typedef struct { struct vdi_ *vdi; } vdi_id_t;

/**
 * struct vdi_color - VDI RGB color
 * @r: red 0..1000
 * @g: green 0..1000
 * @b: blue 0..1000
 */
struct vdi_color {
	uint16_t r;
	uint16_t g;
	uint16_t b;
};

struct vdi_point {
	int16_t x;
	int16_t y;
};

struct vdi_area {
	struct vdi_point a;
	struct vdi_point b;
};

/**
 * enum vdi_cs_type - VDI coordinate system type
 * @VDI_CS_TYPE_NDC: normalized device coordinates
 * @VDI_CS_TYPE_RC: raster coordinates
 */
enum vdi_cs_type {
	VDI_CS_TYPE_NDC = 0,
	VDI_CS_TYPE_RC  = 2,
};

struct vdi_workstation_default {
	union {
		struct {
			int16_t device_id;
			int16_t line_type;
			int16_t line_color;
			int16_t marker_type;
			int16_t marker_color;
			int16_t font;
			int16_t text_color;
			int16_t fill_interior;
			int16_t fill_style;
			int16_t fill_color;
			int16_t cs_type;
		};
		int16_t word[11];
	};
};

struct vdi_workstation {
	union {
		struct {
			int16_t xres;
			int16_t yres;
			int16_t noscale;
			int16_t wpixel;
			int16_t hpixel;
			int16_t cheights;
			int16_t linetypes;
			int16_t linewidths;
			int16_t markertypes;
			int16_t markersizes;
			int16_t faces;
			int16_t patterns;
			int16_t hatches;
			int16_t colors;
			int16_t ngdps;
			int16_t cangdps[10];
			int16_t gdpattr[10];
			int16_t cancolor;
			int16_t cantextrot;
			int16_t canfillarea;
			int16_t cancellarray;
			int16_t palette;
			int16_t locators;
			int16_t valuators;
			int16_t choicedevs;
			int16_t stringdevs;
			int16_t wstype;
			int16_t minwchar;
			int16_t minhchar;
			int16_t maxwchar;
			int16_t maxhchar;
			int16_t minwline;
			int16_t zero5;
			int16_t maxwline;
			int16_t zero7;
			int16_t minwmark;
			int16_t minhmark;
			int16_t maxwmark;
			int16_t maxhmark;
		};
		int16_t word[57];
	};
};

vdi_id_t vdi_v_opnwk(
	const struct vdi_workstation_default *wsd,
	struct vdi_workstation *ws);

static inline bool vdi_id_valid(vdi_id_t vdi_id) { return vdi_id.vdi != NULL; }

void vdi_v_clswk(vdi_id_t vdi_id);

bool vq_color(const vdi_id_t vdi_id, const int index, struct vdi_color *color);

bool vdi_v_fontinit(vdi_id_t vdi_id, const struct fnt *fnt);

#endif /* _GEM_VDI_H */
