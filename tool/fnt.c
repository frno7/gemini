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
	char *format;
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
"    --format <text|bdf>   display format type text (default) or bdf\n"
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
		{ "help",       no_argument,       NULL,               0 },
		{ "version",    no_argument,       NULL,               0 },
		{ "identify",   no_argument,       &option.identify,   1 },
		{ "diagnostic", no_argument,       &option.diagnostic, 1 },
		{ "format",     required_argument, NULL,               0 },
		{ NULL, 0, NULL, 0 }
	};

#define OPT(option) (strcmp(options[index].name, (option)) == 0)

	argv[0] = progname;	/* For better getopt_long error messages. */

	option.format = "text";

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
			else if (OPT("format"))
				option.format = optarg;
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

static void print_fnt_char(const uint16_t c, const struct fnt *fnt)
{
	const int w = fnt_char_width(c, fnt);

	printf("char 0x%x unicode 0x%x %s\n", c,
		charset_atari_st_to_utf32(c, NULL),
		charset_atari_st_to_name(c, NULL));

	printf("char width %d spacing %d\n", w,
		fnt->header->flags.horizontal ?
		fnt_char_horizontal(c, fnt) : 0);

	for (int y = 0; y < fnt->header->bitmap_lines; y++) {
		printf(fnt->header->bitmap_lines <= 10 ?
			"char bitmap %d " : "char bitmap %2d ", y);
		for (int x = 0; x < w; x++)
			printf("%c", fnt_char_pixel(x, y, c, fnt) ? '#' : '.');

		puts("");
	}
}

static void print_fnt_info(const struct fnt *fnt)
{
#define FNT_HEADER_FIELD_PRINT(type_, symbol_, form_)			\
	print_fnt_ ## form_(#symbol_, fnt->header->symbol_);
FNT_HEADER_FIELD(FNT_HEADER_FIELD_PRINT)

	for (uint16_t c = fnt->header->first; c <= fnt->header->last; c++)
		print_fnt_char(c, fnt);
}

static void print_bdf_char(const uint16_t c, const struct fnt *fnt)
{
	const int w = fnt_char_width(c, fnt);

	puts("");

	printf("STARTCHAR %s\n", charset_atari_st_to_name(c, NULL));
	printf("ENCODING %u\n", charset_atari_st_to_utf32(c, NULL));

	printf("SWIDTH %d 0\n", fnt->header->max_char_width*
		72 * 1000 / 75 / fnt->header->point);
	printf("DWIDTH %d 0\n", w);

	printf("BBX %d %d %d %d\n", w,
		fnt->header->bitmap_lines,
		0, -fnt->header->descent);

	puts("BITMAP");
	for (int y = 0; y < fnt->header->bitmap_lines; y++) {
		uint8_t m = 0;

		for (int x = 0; x < w; x++) {
			if (x >= 8 && x % 8 == 0) {
				printf("%02X", m);
				m = 0;
			}

			if (fnt_char_pixel(x, y, c, fnt))
				m |= 0x80 >> (x % 8);
		}

		printf("%02X\n", m);
	}

	puts("ENDCHAR");
}

static void print_bdf(const struct fnt *fnt)
{
	printf("STARTFONT 2.1\n");

	printf("FONT -misc-%s-medium-r-normal--%d-%d-72-72-C-%d-ISO10646-1\n",
		fnt->header->name.s,
		fnt->header->bitmap_lines,
		fnt->header->bitmap_lines * 10, /* Decipoints */
		fnt->header->max_char_width * 10); /* Decipixels */
	printf("SIZE %u 72 72\n",
		fnt->header->bitmap_lines);
	printf("FONTBOUNDINGBOX %d %d %d %d\n",
		fnt->header->max_char_width,
		fnt->header->bitmap_lines,
		0,
		-fnt->header->descent);

	puts("STARTPROPERTIES 17");
	puts("FOUNDRY \"misc\"");
	printf("FAMILY_NAME \"%s\"\n", fnt->header->name.s);
	puts("WEIGHT_NAME \"Medium\"");
	puts("SLANT \"R\"");
	puts("SETWIDTH_NAME \"Normal\"");
	printf("PIXEL_SIZE %d\n",
		fnt->header->bitmap_lines);
	printf("POINT_SIZE %d\n",
		fnt->header->bitmap_lines * 10); /* Decipoints */
	puts("RESOLUTION_X 72");
	puts("RESOLUTION_Y 72");
	printf("SPACING \"%s\"\n", fnt->header->flags.monospace ? "C" : "P");
	printf("AVERAGE_WIDTH %d\n",
		fnt->header->max_char_width * 10); /* Decipixels */
	puts("CHARSET_REGISTRY \"ISO10646\"");
	puts("CHARSET_ENCODING \"1\"");
	/* Bitmap fonts are not copyrightable, hence public domain */
	puts("COPYRIGHT \"Public domain\"");
	puts("DEFAULT_CHAR 32");
	printf("FONT_ASCENT %d\n",
		fnt->header->bitmap_lines -
		fnt->header->descent);
	printf("FONT_DESCENT %d\n",
		fnt->header->descent);
	puts("ENDPROPERTIES");

	printf("\nCHARS %u\n", 1 + fnt->header->last - fnt->header->first);

	for (uint16_t c = fnt->header->first; c <= fnt->header->last; c++)
		print_bdf_char(c, fnt);

	puts("\nENDFONT");
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

	const struct fnt fnt = { .size = f.size, .header = f.data };

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

	if (strcmp(option.format, "text") == 0)
		print_fnt_info(&fnt);
	else if (strcmp(option.format, "bdf") == 0)
		print_bdf(&fnt);
	else
		pr_fatal_error("unrecognised format \"%s\"\n", option.format);

	file_free(&f);

	return EXIT_SUCCESS;

err:
	file_free(&f);

	return EXIT_FAILURE;
}
