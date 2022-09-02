// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "internal/build-assert.h"
#include "internal/file.h"
#include "internal/macro.h"
#include "internal/struct.h"
#include "internal/tiff.h"

#define TIFF_IFD_TAG(t)							\
	t(0x00fe, NEWSUBFILETYPE,           "NewSubfileType")		\
	t(0x0100, IMAGEWIDTH,               "ImageWidth")		\
	t(0x0101, IMAGELENGTH,              "ImageLength")		\
	t(0x0102, BITSPERSAMPLE,            "BitsPerSample")		\
	t(0x0103, COMPRESSION,              "Compression")		\
	t(0x0106, PHOTOMETRICINTERPRETATION,"PhotometricInterpretation")\
	t(0x010d, DOCUMENTNAME,             "DocumentName")		\
	t(0x0111, STRIPOFFSETS,             "StripOffsets")		\
	t(0x0112, ORIENTATION,              "Orientation")		\
	t(0x0115, SAMPLESPERPIXEL,          "SamplesPerPixel")		\
	t(0x0116, ROWSPERSTRIP,             "RowsPerStrip")		\
	t(0x0117, STRIPBYTECOUNTS,          "StripByteCounts")		\
	t(0x0118, MINSAMPLEVALUE,           "MinSampleValue")		\
	t(0x0119, MAXSAMPLEVALUE,           "MaxSampleValue")		\
	t(0x011a, XRESOLUTION,              "XResolution")		\
	t(0x011b, YRESOLUTION,              "YResolution")		\
	t(0x011c, PLANARCONFIGURATION,      "PlanarConfiguration")	\
	t(0x011d, PAGENAME,                 "PageName")			\
	t(0x011e, XPOSITION,                "XPosition")		\
	t(0x011f, YPOSITION,                "YPosition")		\
	t(0x0128, RESOLUTIONUNIT,           "ResolutionUnit")		\
	t(0x0129, PAGENUMBER,               "PageNumber")		\
	t(0x0131, SOFTWARE,                 "Software")			\
	t(0x0132, DATETIME,                 "DateTime")			\
	t(0x014a, SUBIFDS,                  "SubIFDs")			\
	t(0x0152, EXTRASAMPLES,             "ExtraSamples")		\
	t(0x0153, SAMPLEFORMAT,             "SampleFormat")		\
	t(0x8769, EXIF,                     "Exif")			\
	t(0x8773, ICC,                      "ICC")

enum tiff_ifd_tag {
#define TIFF_IFD_TAG_ENUM(id, symbol, string)				\
	TIFF_IFD_TAG_ ## symbol = id,
TIFF_IFD_TAG(TIFF_IFD_TAG_ENUM)
};

#define TIFF_IFD_TYPE(t)						\
	t( 0, 0, UNKNOWN)						\
	t( 1, 1, BYTE)							\
	t( 2, 1, ASCII)							\
	t( 3, 2, SHORT)							\
	t( 4, 4, LONG)							\
	t( 5, 8, RATIONAL)						\
	t( 6, 1, SBYTE)							\
	t( 7, 1, UNDEFINED)						\
	t( 8, 2, SSHORT)						\
	t( 9, 4, SLONG)							\
	t(10, 8, SRATIONAL)						\
	t(11, 4, FLOAT)							\
	t(12, 8, DOUBLE)

enum tiff_ifd_type {
#define TIFF_IFD_TYPE_ENUM(id, size, symbol)				\
	TIFF_IFD_TYPE_ ## symbol = id,
TIFF_IFD_TYPE(TIFF_IFD_TYPE_ENUM)
};

struct tiff_header {
	char endian[2];
	uint16_t magic;
	uint32_t offset;
} PACKED;

struct tiff_ifd_entry {
	uint16_t tag;
	uint16_t type;
	uint32_t count;
	union {
		struct { uint8_t u8[4]; } PACKED;
		struct { uint16_t u16[2]; } PACKED;
		uint32_t u32;
	} PACKED value;
} PACKED;

#define TIFF_IFD_ENTRY_VALUE_SHORT(value_)				\
	.value = { .u32 = (value_) }

#define TIFF_IFD_ENTRY_VALUE_LONG(value_)				\
	.value = { .u32 = (value_) }

#define TIFF_IFD_ENTRY_(tag_, type_, count_, VALUE, value_)		\
	{								\
		.tag = TIFF_IFD_TAG_ ## tag_,				\
		.type = TIFF_IFD_TYPE_ ## type_,			\
		.count = (count_),					\
		VALUE((value_)),					\
	}

#define TIFF_IFD_ENTRY_BITSPERSAMPLE(samples_per_pixel, bits_per_sample)\
	{								\
		.tag = TIFF_IFD_TAG_BITSPERSAMPLE,			\
		.type = TIFF_IFD_TYPE_BYTE,				\
		.count = (samples_per_pixel),				\
		.value = {						\
			.u32 = 						\
				((bits_per_sample) << 24) |		\
				((bits_per_sample) << 16) |		\
				((bits_per_sample) <<  8) |		\
				 (bits_per_sample)			\
		}							\
	}

#define TIFF_IFD_ENTRY_PAGENUMBER(current_, total_)			\
	{								\
		.tag = TIFF_IFD_TAG_PAGENUMBER,				\
		.type = TIFF_IFD_TYPE_SHORT,				\
		.count = 2,						\
		.value = { .u16 = { current_, total_ } }		\
	}

#define TIFF_IFD_ENTRY(tag, type, count, value)				\
	TIFF_IFD_ENTRY_(tag, type, (count),				\
			TIFF_IFD_ENTRY_VALUE_ ## type, (value))

#define TIFF_IFD_ENTRY_SHORT(tag, value)				\
	TIFF_IFD_ENTRY(tag, SHORT, 1, (value))

#define TIFF_IFD_ENTRY_LONG(tag, value)					\
	TIFF_IFD_ENTRY(tag, LONG, 1, (value))

#define TIFF_STRUCT_IFD(N)						\
	struct PACKED tiff_ifd_ ## N {					\
		uint16_t count;						\
		struct tiff_ifd_entry entry[N];				\
		uint32_t offset;					\
	}

static bool tiff_header_ifd(const int i, const int n,
	const uint16_t width, const uint16_t height,
	size_t *offset, const struct tiff_image_f *f, void *arg)
{
	const int samples_per_pixel = 4;
	const size_t data_size = sizeof(struct tiff_pixel[height][width]);

	const char e = BE_LE_SELECT('M', 'I');
	const struct tiff_header header = {
		.endian = { e, e },
		.magic = 42,
		.offset = 8
	};

	const TIFF_STRUCT_IFD(13) ifd = {
		.count = ARRAY_SIZE(ifd.entry),
		.entry = {
			TIFF_IFD_ENTRY_LONG(NEWSUBFILETYPE, n > 1 ? 2 : 0),
			TIFF_IFD_ENTRY_SHORT(IMAGEWIDTH, width),
			TIFF_IFD_ENTRY_SHORT(IMAGELENGTH, height),
			TIFF_IFD_ENTRY_BITSPERSAMPLE(samples_per_pixel, 16),
			TIFF_IFD_ENTRY_SHORT(COMPRESSION, 1),
			TIFF_IFD_ENTRY_SHORT(PHOTOMETRICINTERPRETATION, 2),
			TIFF_IFD_ENTRY_LONG(STRIPOFFSETS, (!*offset ?
				sizeof(header) : *offset) + sizeof(ifd)),
			TIFF_IFD_ENTRY_SHORT(ORIENTATION, 1),
			TIFF_IFD_ENTRY_SHORT(SAMPLESPERPIXEL, samples_per_pixel),
			TIFF_IFD_ENTRY_LONG(ROWSPERSTRIP, height),
			TIFF_IFD_ENTRY_LONG(STRIPBYTECOUNTS, data_size),
			TIFF_IFD_ENTRY_PAGENUMBER(i, n),
			TIFF_IFD_ENTRY_SHORT(EXTRASAMPLES, 1),
		},
		.offset = (i + 1 == n ? 0 :
			(!*offset ? sizeof(header) : *offset) +
			sizeof(ifd) + data_size)
	};

	BUILD_BUG_ON(sizeof(header) != 8);
	BUILD_BUG_ON(sizeof(ifd) != 2 + ARRAY_SIZE(ifd.entry) * 12 + 4);

	if (!*offset) {
		if (!f->write(&header, sizeof(header), arg))
			return false;

		*offset += sizeof(header);
	}

	if (!f->write(&ifd, sizeof(ifd), arg))
		return false;

	*offset += sizeof(ifd) + data_size;

	return true;
}

bool tiff_image(uint16_t n, const struct tiff_image_f *f, void *arg)
{
	size_t offset = 0;

	for (int i = 0; i < n; i++) {
		uint16_t width = 0;
		uint16_t height = 0;

		if (!f->image(&width, &height, arg))
			return false;

		if (!tiff_header_ifd(i, n, width, height, &offset, f, arg))
			return false;

		struct pixel_buf {
			size_t n;
			struct tiff_pixel pixel[1024];
		} buf = { };

		for (uint16_t y = 0; y < height; y++)
		for (uint16_t x = 0; x < width; x++) {
			if (!f->pixel(x, y, &buf.pixel[buf.n++], arg))
				return false;

			if (buf.n < ARRAY_SIZE(buf.pixel))
				continue;

			if (!f->write(buf.pixel,
					sizeof(struct tiff_pixel[buf.n]), arg))
				return false;

			buf = (struct pixel_buf) { };
		}

		if (buf.n && !f->write(buf.pixel,
				sizeof(struct tiff_pixel[buf.n]), arg))
			return false;
	}

	return true;
}

struct file_arg {
	int fd;

	const struct tiff_image_file_f *f;
	void *arg;
};

static bool tiff_file_image(uint16_t *width, uint16_t *height, void *arg)
{
	struct file_arg *arg_ = arg;

	return arg_->f->image(width, height, arg_->arg);
}

static bool tiff_file_pixel(uint16_t x, uint16_t y,
	struct tiff_pixel *pixel, void *arg)
{
	struct file_arg *arg_ = arg;

	return arg_->f->pixel(x, y, pixel, arg_->arg);
}

static bool tiff_file_write(const void *buf, size_t nbyte, void *arg)
{
	struct file_arg *arg_ = arg;

	return xwrite(arg_->fd, buf, nbyte) == nbyte;
}

bool tiff_image_file(const char *path, uint16_t n,
	const struct tiff_image_file_f *f, void *arg)
{
	static const struct tiff_image_f ff = {
		.image = tiff_file_image,
		.pixel = tiff_file_pixel,
		.write = tiff_file_write
	};
	struct file_arg arg_ = {
		.fd = xopen(path, O_WRONLY | O_CREAT | O_TRUNC, 0644),
		.f = f,
		.arg = arg
	};

	if (arg_.fd < 0)
		return false;

	bool valid = tiff_image(n, &ff, &arg_);

	preserve (errno) {
		if (xclose(arg_.fd) < 0)
			valid = false;
	}

	if (!valid)
		goto err;

	return true;

err:
	preserve (errno) {
		unlink(path);
	}

	return false;
}
