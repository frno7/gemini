// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#ifndef INTERNAL_TIFF_H
#define INTERNAL_TIFF_H

#include "types.h"

struct tiff_pixel {
	uint16_t r;
	uint16_t g;
	uint16_t b;
	uint16_t a;
};

struct tiff_image_f {
	bool (*image)(uint16_t *width, uint16_t *height, void *arg);
	bool (*pixel)(uint16_t x, uint16_t y, struct tiff_pixel *pixel, void *arg);
	bool (*write)(const void *buf, size_t nbyte, void *arg);
};

bool tiff_image(uint16_t n, const struct tiff_image_f *f, void *arg);

struct tiff_image_file_f {
	bool (*image)(uint16_t *width, uint16_t *height, void *arg);
	bool (*pixel)(uint16_t x, uint16_t y, struct tiff_pixel *pixel, void *arg);
};

bool tiff_image_file(const char *path, uint16_t n,
	const struct tiff_image_file_f *f, void *arg);

#endif /* INTERNAL_TIFF_H */
