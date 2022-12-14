// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "internal/compare.h"
#include "internal/file.h"
#include "internal/macro.h"
#include "internal/memory.h"
#include "internal/print.h"
#include "internal/string.h"
#include "internal/types.h"

static struct file file_read_fd__(int fd, char *path)
{
	size_t size = 0;
	uint8_t *data = NULL;

	if (fd < 0)
		goto err;

	for (;;) {
		const size_t capacity =
			size + clamp_val(size, 0x1000, 0x100000);

		void * const d = realloc(data, capacity);
		if (!d)
			goto err;
		data = d;

		const ssize_t r = xread(fd, &data[size], capacity - size);
		if (!r) {
			data[size] = '\0';	/* Always NUL terminate */
			break;
		}
		if (r == -1)
			goto err;

		size += r;
	}

	if (xclose(fd) == -1) {
		fd = -1;
		goto err;
	} else
		fd = -1;

	void * const d = realloc(data, size + 1);	/* +1 for NUL */
	if (!d)
		goto err;

	return (struct file) {
		.path = path,
		.size = size,
		.data = d
	};

err:
	preserve (errno) {
		if (fd >= 0)
			xclose(fd);

		free(data);
		free(path);
	}

	return (struct file) { };
}

struct file file_read(const char *path)
{
	return file_read_fd__(xopen(path, O_RDONLY), xstrdup(path));
}

struct file file_read_or_stdin(const char *path)
{
	return strcmp(path, "-") == 0 ?
		file_read_fd(STDIN_FILENO, path) : file_read(path);
}

struct file file_read_fd(int fd, const char *path)
{
	return file_read_fd__(fd, xstrdup(path));
}

bool file_write(const char *path, const void *buf, size_t nbyte)
{
	const int fd = xopen(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if (fd < 0)
		return false;

	const bool valid = (xwrite(fd, buf, nbyte) == nbyte);

	if (xclose(fd) < 0)
		return false;

	return valid;
}

void file_free(struct file *f)
{
	free(f->path);
	free(f->data);
}

bool file_valid(struct file *f)
{
	return f->path != NULL;
}

int xopen(const char *path, int oflag, ...)
{
        mode_t mode = 0;
        va_list ap;

        va_start(ap, oflag);
        if (oflag & O_CREAT)
                mode = va_arg(ap, int);
        va_end(ap);

	do {
                const int fd = open(path, oflag, mode);

                if (fd >= 0)
                        return fd;
	} while (errno == EINTR);

	return -1;
}

int xclose(int fd)
{
	int err;

	do {
		err = close(fd);
	} while (err == EINTR);

	return err < 0 ? -1 : 0;
}

ssize_t xread(int fd, void *buf, size_t nbyte)
{
	uint8_t *data = buf;
	size_t size = 0;

	while (size < nbyte) {
		const ssize_t r = read(fd, &data[size], nbyte - size);

		if (r < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		} else if (!r)
			return size;

		size += r;
	}

	return size;
}

ssize_t xwrite(int fd, const void *buf, size_t nbyte)
{
	const uint8_t *data = buf;
	size_t size = 0;

	while (size < nbyte) {
		const ssize_t w = write(fd, &data[size], nbyte - size);

		if (w < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		} else if (!w)
			return size;

		size += w;
	}

	return size;
}

const char *file_basename(const char *path)
{
	size_t k = 0;

	for (size_t i = 0; path[i]; i++)
		if (path[i] == '/')
			k = i + 1;

	return &path[k];
}
