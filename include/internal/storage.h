// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Fredrik Noring
 */

#ifndef INTERNAL_STORAGE_H
#define INTERNAL_STORAGE_H

#include "internal/types.h"

/**
 * struct storage_file - file storage
 * @path: file path
 * @size: file size in bytes, not counting the NUL termination
 * @data: file data, always NUL terminated
 */
struct storage_file {
	const char *path;
	size_t size;
	void *data;
};

const struct storage_file *storage_files(void);

#define storage_for_each_file(file_)					\
	for ((file_) = storage_files(); (file_)->data; (file_)++)

#endif /* INTERNAL_STORAGE_H */
