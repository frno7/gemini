// SPDX-License-Identifier: GPL-2.0

#include <getopt.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gem/aes.h>
#include <gem/aes-rsc.h>
#include <gem/rsc.h>
#include <gem/rsc-map.h>

#include "internal/assert.h"
#include "internal/build-assert.h"
#include "internal/compare.h"
#include "internal/file.h"
#include "internal/macro.h"
#include "internal/memory.h"
#include "internal/print.h"
#include "internal/tiff.h"

#include "unicode/atari.h"
#include "unicode/utf8.h"

#include "version/version.h"

char progname[] = "rsc";

static struct {
	int map;
	int info;
	int utf8;
	int identify;
	int diagnostic;
	int draw;
	const char *input;
	const char *output;
} option;

static void help(FILE *file)
{
	fprintf(file,
"Usage: %s [options]... <RSC-file>\n"
"\n"
"Displays Atari TOS GEM resource (RSC) file header, strings, images, icons,\n"
"objects, and other details, as text on standard output.\n"
"\n"
"Options:\n"
"\n"
"    -h, --help            display this help and exit\n"
"    --version             display version and exit\n"
"\n"
"    --identify            exit sucessfully if the file is a valid RSC\n"
"    --diagnostic          display diagnostic warnings and errors\n"
"    --encoding <original|utf-8>\n"
"                          display text with original or UTF-8 encoding;\n"
"                          default is UTF-8\n"
"    --map                 display RSC map\n"
"\n"
"    --draw                draw RSC forms and dialogues as images\n"
"    -o, --output <path>   save images as a multipart TIFF file\n"
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
		{ "help",           no_argument, NULL,               0 },
		{ "version",        no_argument, NULL,               0 },
		{ "identify",       no_argument, &option.identify,   1 },
		{ "diagnostic",     no_argument, &option.diagnostic, 1 },
		{ "encoding", required_argument, NULL,               0 },
		{ "map",            no_argument, &option.map,        1 },
		{ "draw",           no_argument, &option.draw,       1 },
		{ "output",   required_argument, NULL,               0 },
		{ NULL, 0, NULL, 0 }
	};

#define OPT(option) (strcmp(options[index].name, (option)) == 0)

	argv[0] = progname;	/* For better getopt_long error messages. */

	option.utf8 = true;

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
			else if (OPT("encoding")) {
				if (strcmp("original", optarg) == 0)
					option.utf8 = false;
				else if (strcmp("utf-8", optarg) == 0)
					option.utf8 = true;
				else
					pr_fatal_error("invalid encoding \"%s\"\n",
						optarg);
			} else if (OPT("output"))
				goto opt_o;
			break;

opt_h:		case 'h':
			help_exit(EXIT_SUCCESS);

opt_o:		case 'o':
			option.output = optarg;
			break;

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

	option.info = !option.map &&
		      !option.draw;
}

static void print_atari_st_char(const char c)
{
	if (option.utf8) {
		uint8_t u[7];	/* Longest UTF-8 sequence with NUL */
		const int k = utf32_to_utf8(
			charset_atari_st_to_utf32(c, NULL), u, sizeof(u));

		if (k > 0) {
			u[k] = '\0';
			printf("%s", u);
		} else
			printf("?");
	} else
		printf("%c", c);
}

static void print_rsc_string_escaped(const char *s)
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

static const char *rsc_object_color_label(const uint8_t color)
{
	switch (color) {
#define RSC_OBJECT_COLOR_LABEL(n_, r_, g_, b_, symbol_, label_, description_)\
	case n_: return #label_;
GEM_PALETTE_COLOR(RSC_OBJECT_COLOR_LABEL)
	default: return "UNDEFINED";
	}
}

static const char *rsc_object_opaque_label(const uint8_t opaque)
{
	return opaque ? "OPAQUE" : "TRANSPARENT";
}

static const char *rsc_tedinfo_font_label(const uint16_t te_font)
{
	switch (te_font) {
#define RSC_TEDINFO_FONT_LABEL(n_, symbol_, label_)			\
	case n_: return #label_;
GEM_TEDINFO_FONT(RSC_TEDINFO_FONT_LABEL)
	default: return "UNDEFINED";
	}
}

static const char *rsc_tedinfo_justificaion_label(const uint16_t te_just)
{
	switch (te_just) {
#define RSC_TEDINFO_JUSTIFICATION_LABEL(n_, symbol_, label_)		\
	case n_: return #label_;
GEM_TEDINFO_JUSTIFICATION(RSC_TEDINFO_JUSTIFICATION_LABEL)
	default: return "UNDEFINED";
	}
}

static void print_rsc_header(const struct rsc_header *h, size_t offset,
	const struct rsc *rsc)
{
#define RSC_HEADER_PRINT(type, name)					\
	printf("header rsh_" #name " %d\n", h->rsh_ ## name);
RSC_HEADER_FIELD(RSC_HEADER_PRINT)
}

static ssize_t rsc_string_indexed(size_t offset, const struct rsc *rsc)
{
	for (ssize_t i = 0; i < rsc->header->rsh_nstring; i++)
		if (rsc_string_offset_at_index(i, rsc) == offset)
			return i;

	return -1;
}

struct rsc_string {
	const char s[1];
};

static void print_rsc_string(const struct rsc_string *string,
	size_t offset, const struct rsc *rsc)
{
	const char *s = (const char *)string;
	const ssize_t i = rsc_string_indexed(offset, rsc);

	if (i < 0)
		printf("string %zd \"", offset);
	else
		printf("frstr %zd \"", i);
	print_rsc_string_escaped(s);
	puts("\"");
}

static void print_rsc_iconblk(const struct rsc_iconblk *iconblk,
	const size_t offset, const struct rsc *rsc)
{
	const int w = iconblk->ib_icon.r.w;
	const int h = iconblk->ib_icon.r.h;

	printf("iconblk %zd icon w %d px h %d px x %d px y %d px fg %s bg %s\n",
		offset, w, h, iconblk->ib_icon.p.x, iconblk->ib_icon.p.y,
		rsc_object_color_label(iconblk->ib_char.color.fg),
		rsc_object_color_label(iconblk->ib_char.color.bg));

	printf("iconblk %zd char x %d px y %d px char 0x%02x", offset,
		iconblk->ib_char.p.x, iconblk->ib_char.p.y,
		iconblk->ib_char.c);
	if (iconblk->ib_char.c) {
		printf(" \"");
		print_atari_st_char(iconblk->ib_char.c);
		puts("\"");
	} else
		puts(" NUL");

	const char *text = rsc_string_at_offset(iconblk->ib_text, rsc);
	BUG_ON(!text);
	printf("iconblk %zd text x %d px y %d px w %d px h %d px string %u \"",
		offset,
		iconblk->ib_txt.p.x, iconblk->ib_txt.p.y,
		iconblk->ib_txt.r.w, iconblk->ib_txt.r.h,
		iconblk->ib_text);
	print_rsc_string_escaped(text);
	puts("\"");

	for (int y = 0; y < h; y++) {
		printf("\t");
		for (int x = 0; x < w; x++) {
			const struct rsc_iconblk_pixel p =
				rsc_iconblk_pixel(x, y, iconblk, rsc);

			printf("%c", p.data ? '#' : p.mask ? 'x' : '.');
		}
		puts("");
	}
}

static ssize_t rsc_bitblk_indexed(size_t offset, const struct rsc *rsc)
{
	for (ssize_t i = 0; i < rsc->header->rsh_nimages; i++)
		if (rsc_frimg_offset_at_index(i, rsc) == offset)
			return i;

	return -1;
}

static void print_rsc_bitblk(const struct rsc_bitblk *bitblk,
	const size_t offset, const struct rsc *rsc)
{
	const ssize_t i = rsc_bitblk_indexed(offset, rsc);
	const int w = bitblk->bi_wb * 8;
	const int h = bitblk->bi_hl;

	if (i < 0)
		printf("bitblk %zd", offset);
	else
		printf("frimg %zd", i);

	printf(" w %d px h %d px x %d px y %d px\n",
		w, h, bitblk->bi_x, bitblk->bi_y);

	for (int y = 0; y < h; y++) {
		printf("\t");
		for (int x = 0; x < w; x++)
			printf("%c", rsc_bitblk_pixel(
				x, y, bitblk, rsc) ? '#' : '.');
		puts("");
	}
}

static void print_rsc_applblk(const struct rsc_applblk *applblk,
	const size_t offset, const struct rsc *rsc)
{
	printf("applblk %zd code %u param %u\n",
		offset, applblk->ap_code, applblk->ap_parm);
}

static void print_rsc_object_color(
	const struct rsc_object_color color, const struct rsc *rsc)
{
	printf("border %s pattern %d %s %s text %s",
		rsc_object_color_label(color.border),
		color.pattern,
		rsc_object_color_label(color.fill),
		rsc_object_opaque_label(color.opaque),
		rsc_object_color_label(color.text));
}

static void print_rsc_tedinfo_integer(const char *name,
	const int n, const struct rsc_tedinfo *tedinfo,
	const size_t tedinfo_offset, const struct rsc *rsc)
{
	printf("tedinfo %zd %s %d \n", tedinfo_offset, name, n);
}

static void print_rsc_tedinfo_string(const char *name,
	const size_t string_offset, const struct rsc_tedinfo *tedinfo,
	const size_t tedinfo_offset, const struct rsc *rsc)
{
	const char *text = rsc_string_at_offset(string_offset, rsc);

	BUG_ON(!text);
	printf("tedinfo %zd %s string %zu \"",
		tedinfo_offset, name, string_offset);
	print_rsc_string_escaped(text);
	puts("\"");
}

static void print_rsc_tedinfo_font(const char *name,
	const int16_t te_font, const struct rsc_tedinfo *tedinfo,
	const size_t tedinfo_offset, const struct rsc *rsc)
{
	printf("tedinfo %zd %s %d %s\n", tedinfo_offset, name,
		te_font, rsc_tedinfo_font_label(te_font));
}

static void print_rsc_tedinfo_just(const char *name,
	const int16_t te_just, const struct rsc_tedinfo *tedinfo,
	const size_t tedinfo_offset, const struct rsc *rsc)
{
	printf("tedinfo %zd %s %d %s\n", tedinfo_offset, name,
		te_just, rsc_tedinfo_justificaion_label(te_just));
}

static void print_rsc_tedinfo_color(const char *name,
	const struct rsc_object_color te_color,
	const struct rsc_tedinfo *tedinfo, const size_t tedinfo_offset,
	const struct rsc *rsc)
{
	printf("tedinfo %zd %s ", tedinfo_offset, name);
	print_rsc_object_color(te_color, rsc);
	puts("");
}

static void print_rsc_tedinfo(const struct rsc_tedinfo *tedinfo,
	const size_t offset, const struct rsc *rsc)
{
#define RSC_TEDINFO_PRINT(type, name, form)				\
	print_rsc_tedinfo_ ## form("te_" #name,				\
		tedinfo->te_ ## name, tedinfo, offset, rsc);
RSC_TEDINFO_FIELD(RSC_TEDINFO_PRINT)
}

static void print_rsc_object_prefix(const int16_t ob,
	const struct rsc_object *tree, const char *prefix)
{
	if (rsc_valid_ob(ob)) {
		print_rsc_object_prefix(rsc_object_parent(ob, tree),
			tree, prefix);

		printf(" object %d", ob);
	} else
		printf("%s", prefix);
}

static void print_rsc_object_link(
	const struct rsc_object_link link,
	const int16_t ob,
	const struct rsc_object *tree,
	const struct rsc *rsc,
	const char *prefix)
{
	print_rsc_object_prefix(ob, tree, prefix);
	printf(" link next %d\n", link.next);

	print_rsc_object_prefix(ob, tree, prefix);
	printf(" link head %d\n", link.head);

	print_rsc_object_prefix(ob, tree, prefix);
	printf(" link tail %d\n", link.tail);
}

static const char *rsc_object_g_type_label(const struct rsc_object_type type)
{
	switch (type.g) {
#define RSC_OBJECT_G_TYPE_PRINT(n_, symbol_, label_, spec_)		\
	case n_: return "G_" #label_;
GEM_OBJECT_G_TYPE(RSC_OBJECT_G_TYPE_PRINT)
	default: return "";
	}
}

static void print_rsc_object_type(
	const struct rsc_object_type type,
	const int16_t ob,
	const struct rsc_object *tree,
	const struct rsc *rsc,
	const char *prefix)
{
	const char *label = rsc_object_g_type_label(type);

	print_rsc_object_prefix(ob, tree, prefix);
	printf(" shape type %d %s %d\n", type.g,
		*label ? label : "G_UNDEFINED", type.m);
}

static void print_rsc_object_flags(
	const struct rsc_object_flags flags,
	const int16_t ob,
	const struct rsc_object *tree,
	const struct rsc *rsc,
	const char *prefix)
{
	print_rsc_object_prefix(ob, tree, prefix);
	printf(" shape flags 0x%x", flags.mask);

#define RSC_OBJECT_FLAG_PRINT(bit_, symbol_, label_)			\
	if (flags.symbol_)						\
		printf(" %s", #label_);
GEM_OBJECT_FLAG(RSC_OBJECT_FLAG_PRINT)

	if (flags.undefined)
		printf(" UNDEFINED_0x%x", flags.undefined);

	puts("");
}

static void print_rsc_object_state(
	const struct rsc_object_state state,
	const int16_t ob,
	const struct rsc_object *tree,
	const struct rsc *rsc,
	const char *prefix)
{
	print_rsc_object_prefix(ob, tree, prefix);
	printf(" shape state 0x%x", state.mask);

#define RSC_OBJECT_STATE_PRINT(bit_, symbol_, label_)			\
	if (state.symbol_)						\
		printf(" %s", #label_);
GEM_OBJECT_STATE(RSC_OBJECT_STATE_PRINT)

	if (state.undefined)
		printf(" UNDEFINED_0x%x", state.undefined);

	puts("");
}

static void print_rsc_object_spec_box(
	const struct rsc_object_spec spec, const struct rsc *rsc)
{
	printf(" box %d px ", spec.box.thickness);
	print_rsc_object_color(spec.box.color, rsc);
	puts("");
}

static void print_rsc_object_spec_string(
	const struct rsc_object_spec spec, const struct rsc *rsc)
{
	const char *s = rsc_string_at_offset(spec.string, rsc);

	BUG_ON(!s);

	printf(" string %u \"", spec.string);
	print_rsc_string_escaped(s);
	puts("\"");
}

static void print_rsc_object_spec_bitblk(
	const struct rsc_object_spec spec, const struct rsc *rsc)
{
	printf(" bitblk %u\n", spec.bitblk);
}

static void print_rsc_object_spec_iconblk(
	const struct rsc_object_spec spec, const struct rsc *rsc)
{
	printf(" iconblk %u\n", spec.iconblk);
}

static void print_rsc_object_spec_ciconblk(
	const struct rsc_object_spec spec, const struct rsc *rsc)
{
	printf(" ciconblk %u\n", spec.ciconblk);
}

static void print_rsc_object_spec_tedinfo(
	const struct rsc_object_spec spec, const struct rsc *rsc)
{
	printf(" tedinfo %u\n", spec.tedinfo);
}

static void print_rsc_object_spec_applblk(
	const struct rsc_object_spec spec, const struct rsc *rsc)
{
	printf(" applblk %u\n", spec.applblk);
}

static void print_rsc_object_spec(
	const struct rsc_object_spec spec,
	const int16_t ob,
	const struct rsc_object *tree,
	const struct rsc *rsc,
	const char *prefix)
{
	print_rsc_object_prefix(ob, tree, prefix);
	printf(" shape spec");

	switch (tree[ob].shape.type.g) {
#define RSC_OBJECT_G_TYPE_SPEC(n_, symbol_, label_, spec_)		\
	case n_: return print_rsc_object_spec_ ## spec_(spec, rsc);
GEM_OBJECT_G_TYPE(RSC_OBJECT_G_TYPE_SPEC)
	default: printf(" UNDEFINED_0x%x\n", spec.undefined);
	}
}

static void print_rsc_object_area(
	const struct rsc_object_area area,
	const int16_t ob,
	const struct rsc_object *tree,
	const struct rsc *rsc,
	const char *prefix)
{
	print_rsc_object_prefix(ob, tree, prefix);
	printf(" shape area x %d ch %d px", area.p.x.ch, area.p.x.px);
	printf(" y %d ch %d px\n", area.p.y.ch, area.p.y.px);

	print_rsc_object_prefix(ob, tree, prefix);
	printf(" shape area w %d ch %d px", area.r.w.ch, area.r.w.px);
	printf(" h %d ch %d px\n", area.r.h.ch, area.r.h.px);
}

static void print_rsc_object(const int16_t ob,
	const struct rsc_object *tree, const struct rsc *rsc,
	const char *prefix)
{
	print_rsc_object_link (tree[ob].link,       ob, tree, rsc, prefix);
	print_rsc_object_type (tree[ob].shape.type,  ob, tree, rsc, prefix);
	print_rsc_object_flags(tree[ob].shape.flags, ob, tree, rsc, prefix);
	print_rsc_object_state(tree[ob].shape.state, ob, tree, rsc, prefix);
	print_rsc_object_spec (tree[ob].shape.spec,  ob, tree, rsc, prefix);
	print_rsc_object_area (tree[ob].shape.area,  ob, tree, rsc, prefix);
}

static void print_rsc_tree(const size_t i, const struct rsc *rsc)
{
	const struct rsc_object *tree = rsc_tree_at_index(i, rsc);
	char prefix[32];

	BUG_ON(!tree);

	snprintf(prefix, sizeof(prefix), "tree %zu", i);

	for (int16_t ob = 0; rsc_valid_ob(ob); ob = rsc_tree_traverse(ob, tree))
		print_rsc_object(ob, tree, rsc, prefix);
}

static void print_rsc_trees(const struct rsc *rsc)
{
	for (size_t i = 0; i < rsc->header->rsh_ntree; i++)
		print_rsc_tree(i, rsc);
}

static void print_rsc_info(const struct rsc *rsc)
{
	struct rsc_map *map = rsc_map_alloc(rsc);
	struct rsc_map_region region;

	BUG_ON(!map);

	if (!rsc_map(map, rsc))
		pr_fatal_error("%s: malformed RSC structure\n", option.input);

#define PRINT_INFO_TYPE(type_)						\
	rsc_map_for_each_region (region, map)				\
		if (region.entry.type == rsc_map_entry_type_ ## type_ &&\
		    !region.entry.reserved) {				\
			const uint8_t *b = (const uint8_t *)rsc->header;\
			const void *v = &b[region.offset];		\
			print_rsc_ ## type_(				\
				(const struct rsc_ ## type_ *)v,	\
				region.offset, rsc);			\
		}

	PRINT_INFO_TYPE(header)
	PRINT_INFO_TYPE(string)
	PRINT_INFO_TYPE(iconblk)
	PRINT_INFO_TYPE(bitblk)
	PRINT_INFO_TYPE(applblk)
	PRINT_INFO_TYPE(tedinfo)

	rsc_map_free(map);

	print_rsc_trees(rsc);
}

static void print_rsc_map_region(
	const struct rsc_map_region *region, const struct rsc *rsc)
{
	const char *s = rsc_map_entry_type_symbol(region->entry.type);

	printf("%-8s %5zu %zu%s\n",
		s[0] ? s : "-",
		region->offset, region->size,
		region->entry.reserved ? " reserved" : "");

	if (region->entry.type == rsc_map_entry_type_unused ||
	    region->entry.reserved) {
		const uint8_t *b = (const uint8_t *)rsc->header;

		pr_mem(stdout, &b[region->offset], region->size);
	}
}

static bool print_rsc_map(const struct rsc *rsc)
{
	struct rsc_map *map = rsc_map_alloc(rsc);

	BUG_ON(!map);

	if (!rsc_map(map, rsc)) {
		rsc_map_free(map);

		return false;
	}

	struct rsc_map_region region;

	rsc_map_for_each_region (region, map)
		print_rsc_map_region(&region, rsc);

	rsc_map_free(map);

	return true;
}

struct draw_rsc_arg {
	int i;
	aes_id_t aes_id;
	const struct rsc_object *tree;
	const struct rsc *rsc;
};

static bool draw_rsc_image(uint16_t *width, uint16_t *height, void *arg_)
{
	struct draw_rsc_arg *arg = arg_;

	arg->tree = rsc_tree_at_index(arg->i++, arg->rsc);

	const struct aes_area bounds =
		aes_rsc_tree_bounds(arg->aes_id, arg->tree, arg->rsc);

	*width = bounds.r.w;
	*height = bounds.r.h;

	return true;
}

static bool draw_rsc_pixel(uint16_t x, uint16_t y,
	struct tiff_pixel *pixel, void *arg_)
{
	struct draw_rsc_arg *arg = arg_;
	struct vdi_color color;

	if (!aes_palette_color(arg->aes_id, aes_objc_pixel(arg->aes_id,
			(struct aes_point) { .x = x, .y = y },
			arg->tree, arg->rsc), &color))
		return true;

	pixel->r = (0xffff * color.r + 500) / 1000;
	pixel->g = (0xffff * color.g + 500) / 1000;
	pixel->b = (0xffff * color.b + 500) / 1000;
	pixel->a =  0xffff;

	return true;
}

static bool draw_rsc(const struct rsc *rsc)
{
	struct aes aes_ = { };
	struct draw_rsc_arg arg = {
		.aes_id = aes_appl_init(&aes_),
		.rsc = rsc
	};
	const struct tiff_image_file_f f = {
		.image = draw_rsc_image,
		.pixel = draw_rsc_pixel
	};

	if (!aes_id_valid(arg.aes_id))
		pr_fatal_error("%s: Failed to open AES\n", option.input);

	if (!tiff_image_file(option.output, rsc->header->rsh_ntree, &f, &arg))
		pr_fatal_errno(option.output);

	aes_appl_exit(arg.aes_id);

	return true;
}

static void print_rsc_warning(const char *msg, void *arg)
{
	dprintf(STDERR_FILENO, "%s: warning: %s\n", option.input, msg);
}

static void print_rsc_error(const char *msg, void *arg)
{
	dprintf(STDERR_FILENO, "%s: error: %s\n", option.input, msg);
}

int main(int argc, char *argv[])
{
	parse_options(argc, argv);

	struct file f = file_read(option.input);

	if (!file_valid(&f))
		pr_fatal_errno(option.input);

	const struct rsc rsc = {
		.size = f.size,
		.header = (struct rsc_header *)f.data
	};

	if (option.identify) {
		const bool valid = rsc_valid_structure(&rsc);

		file_free(&f);

		return valid ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	static const struct rsc_diagnostic print_rsc_diagnostic = {
		.warning = print_rsc_warning,
		.error   = print_rsc_error,
	};

	if (option.diagnostic) {
		const bool valid = rsc_valid_structure_diagnostic(
			&rsc, &print_rsc_diagnostic, NULL);

		file_free(&f);

		return valid ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	if (!rsc_valid_structure_diagnostic(&rsc, &print_rsc_diagnostic, NULL))
		goto err;

	if (option.info)
		print_rsc_info(&rsc);

	if (option.draw && !draw_rsc(&rsc))
		goto err;

	if (option.map && !print_rsc_map(&rsc)) {
		pr_fatal_error("%s: malformed RSC structure\n", option.input);

		goto err;
	}

	file_free(&f);

	return EXIT_SUCCESS;

err:
	file_free(&f);

	return EXIT_FAILURE;
}
