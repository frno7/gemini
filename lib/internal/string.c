// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "internal/assert.h"
#include "internal/memory.h"
#include "internal/string.h"
#include "internal/types.h"

char *xstrdup(const char *s)
{
	void *t = strdup(s);

	if (!t)
		pr_fatal_errno("strdup");

	return t;
}

char *xstrcat(const char *a, const char *b)
{
	const size_t alen = strlen(a);
	const size_t blen = strlen(b);
	char *s = xmalloc(alen + blen + 1);

	memcpy(&s[0], a, alen);
	memcpy(&s[alen], b, blen);
	s[alen + blen] = '\0';

	return s;
}

char *xstrndup(const char *s, size_t n)
{
	void *t = strndup(s, n);

	if (!t)
		pr_fatal_errno("strndup");

	return t;
}

bool strtoint(int *n, const char *s, int base)
{
	char *e;

	errno = 0;
	const long int m = strtol(s, &e, base);

	if (e != &s[strlen(s)] || errno == ERANGE)
		return false;

	if (m < INT_MIN || INT_MAX < m)
		return false;

	*n = m;

	return true;
}

bool strsuffix(const char *suffix, const char *s)
{
	const size_t suffix_length = strlen(suffix);
	const size_t s_length = strlen(s);

	return suffix_length <= s_length &&
		strcmp(suffix, &s[s_length - suffix_length]) == 0;
}

struct string_split first_string_split(
	const char *s, const char *sep,
	char *(find)(const char *s, const char *sep))
{
	if (s[0] == '\0')
		return (struct string_split) {
			.length = 0,
			.s = s,
			.eos = true
		};

	if (sep[0] == '\0')
		return (struct string_split) {
			.length = 1,
			.s = s,
			.eos = (s[1] == '\0')
		};

	const char *t = find(s, sep);

	if (!t)
		return (struct string_split) {
			.length = strlen(s),
			.s = s,
			.eos = true
		};

	const size_t n = t - s;

	if (!n) {
		const size_t sep_len = strlen(sep);

		return (struct string_split) {
			.length = sep_len,
			.s = s,
			.sep = true,
			.eos = (s[sep_len] == '\0')
		};
	}

	return (struct string_split) {
		.length = n,
		.s = s,
		.eos = (s[n] == '\0')
	};
}

struct string_split next_string_split(
	struct string_split split, const char *sep,
	char *(find)(const char *s, const char *sep))
{
	return split.eos ? (struct string_split) { } :
		first_string_split(&split.s[split.length], sep, find);
}

struct line_column char_line_column(char c, struct line_column lc)
{
	if (c == '\n') {
		lc.line++;
		lc.column = 1;
	} else
		lc.column = 1 + (c == '\t' ?
			(((lc.column - 1) >> 3) << 3) + 8 : lc.column);

	return lc;
}

struct line_column string_line_column(const char *s, struct line_column lc)
{
	for (size_t i = 0; s[i] != '\0'; i++)
		lc = char_line_column(s[i], lc);

	return lc;
}

bool sbprintf(struct strbuf *sb, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	const int length = vsbprintf(sb, fmt, ap);
	va_end(ap);

	return length;
}

bool sbmprintf(struct strbuf *sb, size_t margin, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	const int length = vsbmprintf(sb, margin, fmt, ap);
	va_end(ap);

	return length;
}

bool vsbprintf(struct strbuf *sb, const char *fmt, va_list ap)
{
	return vsbmprintf(sb, 4096, fmt, ap);
}

bool vsbmprintf(struct strbuf *sb, size_t margin, const char *fmt, va_list ap)
{
	if (!margin)
		return false;

	if (sb->capacity < sb->length + margin) {
		const size_t capacity = sb->length + 2 * margin;
		char *s = realloc(sb->s, capacity);

		if (!s)
			return false;

		sb->capacity = capacity;
		sb->s = s;
	}

	const int length = vsnprintf(&sb->s[sb->length],
		sb->capacity - sb->length, fmt, ap);

	if (length < 0 || sb->capacity <= sb->length + length + 1) {
		sb->s[sb->length] = '\0';
		return false;
	}

	sb->length += length;

	return true;
}
