// SPDX-License-Identifier: GPL-2.0

#ifndef INTERNAL_PRINT_H
#define INTERNAL_PRINT_H

#include <stdarg.h>
#include <stdio.h>

#include "internal/macro.h"

void pr_warn(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

void pr_warn_errno(const char *s);

void pr_errno(const char *s);

void pr_error(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

void NORETURN pr_fatal_error(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

void NORETURN pr_fatal_errno(const char *s);

void pr_bug_warn(const char *file, int line,
	const char *func, const char *fmt, ...);

void NORETURN pr_bug(const char *file, int line,
	const char *func, const char *expr);

void pr_mem(FILE *f, const void *data, size_t size);

void report_msg(void (*f)(const char *msg, void *arg), void *arg,
	const char *prefix, const char *suffix, const char *fmt, va_list ap);

#endif /* INTERNAL_PRINT_H */
