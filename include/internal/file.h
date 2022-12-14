// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#ifndef INTERNAL_FILE_H
#define INTERNAL_FILE_H

#include "internal/types.h"

/**
 * struct file - file container
 * @path: path of file
 * @size: size in bytes of file
 * @data: contents of file, always NUL terminated
 */
struct file {
	char * path;
	size_t size;
	void * data;
};

struct file file_read(const char *path);

struct file file_read_or_stdin(const char *path);

struct file file_read_fd(int fd, const char *path);

bool file_write(const char *path, const void *buf, size_t nbyte);

void file_free(struct file *f);

bool file_valid(struct file *f);

int xopen(const char *path, int oflag, ...);

int xclose(int fd);

ssize_t xread(int fd, void *buf, size_t nbyte);

ssize_t xwrite(int fd, const void *buf, size_t nbyte);

const char *file_basename(const char *path);

#endif /* INTERNAL_FILE_H */
