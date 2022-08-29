// SPDX-License-Identifier: GPL-2.0

#include <getopt.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gem/fnt.h>

#include "internal/file.h"
#include "internal/macro.h"
#include "internal/print.h"

#include "unicode/atari.h"
#include "unicode/utf8.h"

#include "version/version.h"

char progname[] = "fnt";

static struct {
	int identify;
	int diagnostic;
	const char *input;
} option;

static void help(FILE *file)
{
	fprintf(file,
"Usage: %s [options]... <FNT-file>\n"
"\n"
"Displays Atari TOS GEM font (FNT) file header and character set, as text\n"
"on standard output.\n"
"\n"
"Options:\n"
"\n"
"    -h, --help            display this help and exit\n"
"    --version             display version and exit\n"
"\n"
"    --identify            exit sucessfully if the file is a valid FNT\n"
"    --diagnostic          display diagnostic warnings and errors\n"
"\n",
		progname);
}

static void NORETURN help_exit(int code)
{
	help(stdout);

	exit(code);
}

static void NORETURN version_exit(void)
{
	printf("%s version %s\n", progname, gemini_version());

	exit(EXIT_SUCCESS);
}

static void parse_options(int argc, char **argv)
{
	static const struct option options[] = {
		{ "help",       no_argument, NULL,               0 },
		{ "version",    no_argument, NULL,               0 },
		{ "identify",   no_argument, &option.identify,   1 },
		{ "diagnostic", no_argument, &option.diagnostic, 1 },
		{ NULL, 0, NULL, 0 }
	};

#define OPT(option) (strcmp(options[index].name, (option)) == 0)

	argv[0] = progname;	/* For better getopt_long error messages. */

	for (;;) {
		int index = 0;

		switch (getopt_long(argc, argv, "ho:", options, &index)) {
		case -1:
			goto out;

		case 0:
			if (OPT("help"))
				goto opt_h;
			else if (OPT("version"))
				version_exit();
			break;

opt_h:		case 'h':
			help_exit(EXIT_SUCCESS);

		case '?':
			exit(EXIT_FAILURE);
		}
	}

#undef OPT
out:

	if (optind == argc)
		pr_fatal_error("missing input file\n");
	if (optind + 1 < argc)
		pr_fatal_error("%s: too many input files\n", argv[optind + 1]);

	option.input = argv[optind];
}

static void print_atari_st_char(const char c)
{
	uint8_t u[7];	/* Longest UTF-8 sequence with NUL */
	const int k = utf32_to_utf8(
		charset_atari_st_to_utf32(c, NULL), u, sizeof(u));

	if (k > 0) {
		u[k] = '\0';
		printf("%s", u);
	} else
		printf("?");
}

static void print_fnt_string_escaped(const char *s)
{
	for (size_t i = 0; s[i] != '\0'; i++)
		switch (s[i]) {
		case '\0': printf("\\0");  break;
		case '\t': printf("\\t");  break;
		case '\r': printf("\\r");  break;
		case '\n': printf("\\n");  break;
		case '\\': printf("\\\\"); break;
		case  '"': printf("\\\""); break;
		default:   print_atari_st_char(s[i]);
		}
}

static void print_fnt_integer(const char *symbol, const uint32_t n)
{
	printf("header %s %d\n", symbol, n);
}

static void print_fnt_name(const char *symbol,
	const struct fnt_header_name name)
{
	printf("header %s \"", symbol);
	print_fnt_string_escaped(name.s);
	puts("\"");
}

static void print_fnt_mask(const char *symbol, const uint32_t mask)
{
	printf("header %s 0x%x\n", symbol, mask);
}

static void print_fnt_flags(const char *symbol,
	const struct fnt_header_flags flags)
{
#define FNT_HEADER_FLAGS_PRINT(n_, symbol_)				\
	printf("header %s " #symbol_ " %d\n", symbol, flags.symbol_);
FNT_HEADER_FLAGS_FIELD(FNT_HEADER_FLAGS_PRINT)
	printf("header %s undefined %x\n", symbol, flags.undefined);
}

static int fnt_char_width(const uint16_t c, const struct fnt *fnt)
{
	const struct fnt_header *h = fnt->data;

	if (c < h->first || c > h->last)
		return -1;

	const int32_t x0 = fnt_char_offset(c + 0, fnt);
	const int32_t x1 = fnt_char_offset(c + 1, fnt);

	return x0 >= 0 && x1 >= 0 && x0 <= x1 ? x1 - x0 : -1;
}

static bool fnt_char_pixel(const int x, const int y,
	const uint16_t c, const struct fnt *fnt)
{
	const struct fnt_header *h = fnt->data;

	const int w = fnt_char_width(c, fnt);

	if (w <= 0)
		return false;

	if (c < h->first || c > h->last ||
	    x < 0 || x >= w ||
	    y < 0 || y >= h->bitmap_lines)
		return false;

	const int32_t x0 = fnt_char_offset(c, fnt);

	if (x0 < 0)
		return false;

	/* FIXME: Consider h->flags.big_endian */

	const uint8_t *b = (const uint8_t *)h;
	const uint8_t *d = &b[h->character_bitmap +
			      h->bitmap_stride * y + ((x0 + x) / 8)];

	return (*d & (0x80 >> ((x0 + x) % 8))) != 0;
}

static void print_fnt_char(const uint16_t c, const struct fnt *fnt)
{
	const struct fnt_header *h = fnt->data;

	const int w = fnt_char_width(c, fnt);

	printf("char 0x%x unicode 0x%x\n",
		c, charset_atari_st_to_utf32(c, NULL));

	printf("char width %d spacing %d\n", w,
		h->flags.horizontal ? fnt_char_horizontal(c, fnt) : 0);

	for (int y = 0; y < h->bitmap_lines; y++) {
		printf(h->bitmap_lines <= 10 ?
			"char bitmap %d " : "char bitmap %2d ", y);
		for (int x = 0; x < w; x++)
			printf("%c", fnt_char_pixel(x, y, c, fnt) ? '#' : '.');

		puts("");
	}
}

static void print_fnt_info(const struct fnt *fnt)
{
	const struct fnt_header *h = fnt->data;

#define FNT_HEADER_FIELD_PRINT(type_, symbol_, form_)			\
	print_fnt_ ## form_(#symbol_, h->symbol_);
FNT_HEADER_FIELD(FNT_HEADER_FIELD_PRINT)

	for (uint16_t c = h->first; c <= h->last; c++)
		print_fnt_char(c, fnt);
}

static void print_fnt_warning(const char *msg, void *arg)
{
	dprintf(STDERR_FILENO, "%s: warning: %s\n", option.input, msg);
}

static void print_fnt_error(const char *msg, void *arg)
{
	dprintf(STDERR_FILENO, "%s: error: %s\n", option.input, msg);
}

int main(int argc, char *argv[])
{
	parse_options(argc, argv);

	struct file f = file_read(option.input);

	if (!file_valid(&f))
		pr_fatal_errno(f.path);

	const struct fnt fnt = { .size = f.size, .data = f.data };

	if (option.identify) {
		const bool valid = fnt_valid(&fnt);

		file_free(&f);

		return valid ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	static const struct fnt_diagnostic print_fnt_diagnostic = {
		.warning = print_fnt_warning,
		.error   = print_fnt_error,
	};

	if (option.identify) {
		const bool valid = fnt_valid_diagnostic(
			&fnt, &print_fnt_diagnostic, NULL);

		file_free(&f);

		return valid ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	if (!fnt_valid_diagnostic(&fnt, &print_fnt_diagnostic, NULL))
		goto err;

	print_fnt_info(&fnt);

	file_free(&f);

	return EXIT_SUCCESS;

err:
	file_free(&f);

	return EXIT_FAILURE;
}
