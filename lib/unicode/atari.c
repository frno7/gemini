// SPDX-License-Identifier: GPL-2.0

#include "internal/macro.h"

#include "unicode/atari.h"
#include "unicode/utf8.h"

static const uint16_t atari_st_charset[256] = {
	0x0000,	// <NUL>	NULL
	0x0001,	// <SOH>	START OF HEADING
	0x0002,	// <STX>	START OF TEXT
	0x0003,	// <ETX>	END OF TEXT
	0x0004,	// <EOT>	END OF TRANSMISSION
	0x0005,	// <ENQ>	ENQUIRY
	0x0006,	// <ACK>	ACKNOWLEDGE
	0x0007,	// <BEL>	BELL
	0x0008,	// <BS>		BACKSPACE
	0x0009,	// <TAB>	HORIZONTAL TABULATION
	0x000a,	// <LF>		LINE FEED
	0x000b,	// <VT>		VERTICAL TABULATION
	0x000c,	// <FF>		FORM FEED
	0x000d,	// <CR>		CARRIAGE RETURN
	0x000e,	// <SO>		SHIFT OUT
	0x000f,	// <SI>		SHIFT IN
	0x0010,	// <DLE>	DATA LINK ESCAPE
	0x0011,	// <DC1>	DEVICE CONTROL ONE
	0x0012,	// <DC2>	DEVICE CONTROL TWO
	0x0013,	// <DC3>	DEVICE CONTROL THREE
	0x0014,	// <DC4>	DEVICE CONTROL FOUR
	0x0015,	// <NAK>	NEGATIVE ACKNOWLEDGE
	0x0016,	// <SYN>	SYNCHRONOUS IDLE
	0x0017,	// <ETB>	END OF TRANSMISSION BLOCK
	0x0018,	// <CAN>	CANCEL
	0x0019,	// <EM>		END OF MEDIUM
	0x001a,	// <SUB>	SUBSTITUTE
	0x001b,	// <ESC>	ESCAPE
	0x001c,	// <FS>		FILE SEPARATOR
	0x001d,	// <GS>		GROUP SEPARATOR
	0x001e,	// <RS>		RECORD SEPARATOR
	0x001f,	// <US>		UNIT SEPARATOR
	0x0020,	// <SP>		SPACE
	0x0021,	// !		EXCLAMATION MARK
	0x0022,	// "		QUOTATION MARK
	0x0023,	// #		NUMBER SIGN
	0x0024,	// $		DOLLAR SIGN
	0x0025,	// %		PERCENT SIGN
	0x0026,	// &		AMPERSAND
	0x0027,	// '		APOSTROPHE
	0x0028,	// (		LEFT PARENTHESIS
	0x0029,	// )		RIGHT PARENTHESIS
	0x002a,	// *		ASTERISK
	0x002b,	// +		PLUS SIGN
	0x002c,	// ,		COMMA
	0x002d,	// -		HYPHEN-MINUS
	0x002e,	// .		FULL STOP
	0x002f,	// /		SOLIDUS
	0x0030,	// 0		DIGIT ZERO
	0x0031,	// 1		DIGIT ONE
	0x0032,	// 2		DIGIT TWO
	0x0033,	// 3		DIGIT THREE
	0x0034,	// 4		DIGIT FOUR
	0x0035,	// 5		DIGIT FIVE
	0x0036,	// 6		DIGIT SIX
	0x0037,	// 7		DIGIT SEVEN
	0x0038,	// 8		DIGIT EIGHT
	0x0039,	// 9		DIGIT NINE
	0x003a,	// :		COLON
	0x003b,	// ;		SEMICOLON
	0x003c,	// <		LESS-THAN SIGN
	0x003d,	// =		EQUALS SIGN
	0x003e,	// >		GREATER-THAN SIGN
	0x003f,	// ?		QUESTION MARK
	0x0040,	// @		COMMERCIAL AT
	0x0041,	// A		LATIN CAPITAL LETTER A
	0x0042,	// B		LATIN CAPITAL LETTER B
	0x0043,	// C		LATIN CAPITAL LETTER C
	0x0044,	// D		LATIN CAPITAL LETTER D
	0x0045,	// E		LATIN CAPITAL LETTER E
	0x0046,	// F		LATIN CAPITAL LETTER F
	0x0047,	// G		LATIN CAPITAL LETTER G
	0x0048,	// H		LATIN CAPITAL LETTER H
	0x0049,	// I		LATIN CAPITAL LETTER I
	0x004a,	// J		LATIN CAPITAL LETTER J
	0x004b,	// K		LATIN CAPITAL LETTER K
	0x004c,	// L		LATIN CAPITAL LETTER L
	0x004d,	// M		LATIN CAPITAL LETTER M
	0x004e,	// N		LATIN CAPITAL LETTER N
	0x004f,	// O		LATIN CAPITAL LETTER O
	0x0050,	// P		LATIN CAPITAL LETTER P
	0x0051,	// Q		LATIN CAPITAL LETTER Q
	0x0052,	// R		LATIN CAPITAL LETTER R
	0x0053,	// S		LATIN CAPITAL LETTER S
	0x0054,	// T		LATIN CAPITAL LETTER T
	0x0055,	// U		LATIN CAPITAL LETTER U
	0x0056,	// V		LATIN CAPITAL LETTER V
	0x0057,	// W		LATIN CAPITAL LETTER W
	0x0058,	// X		LATIN CAPITAL LETTER X
	0x0059,	// Y		LATIN CAPITAL LETTER Y
	0x005a,	// Z		LATIN CAPITAL LETTER Z
	0x005b,	// [		LEFT SQUARE BRACKET
	0x005c,	// \		REVERSE SOLIDUS
	0x005d,	// ]		RIGHT SQUARE BRACKET
	0x005e,	// ^		CIRCUMFLEX ACCENT
	0x005f,	// _		LOW LINE
	0x0060,	// `		GRAVE ACCENT
	0x0061,	// a		LATIN SMALL LETTER A
	0x0062,	// b		LATIN SMALL LETTER B
	0x0063,	// c		LATIN SMALL LETTER C
	0x0064,	// d		LATIN SMALL LETTER D
	0x0065,	// e		LATIN SMALL LETTER E
	0x0066,	// f		LATIN SMALL LETTER F
	0x0067,	// g		LATIN SMALL LETTER G
	0x0068,	// h		LATIN SMALL LETTER H
	0x0069,	// i		LATIN SMALL LETTER I
	0x006a,	// j		LATIN SMALL LETTER J
	0x006b,	// k		LATIN SMALL LETTER K
	0x006c,	// l		LATIN SMALL LETTER L
	0x006d,	// m		LATIN SMALL LETTER M
	0x006e,	// n		LATIN SMALL LETTER N
	0x006f,	// o		LATIN SMALL LETTER O
	0x0070,	// p		LATIN SMALL LETTER P
	0x0071,	// q		LATIN SMALL LETTER Q
	0x0072,	// r		LATIN SMALL LETTER R
	0x0073,	// s		LATIN SMALL LETTER S
	0x0074,	// t		LATIN SMALL LETTER T
	0x0075,	// u		LATIN SMALL LETTER U
	0x0076,	// v		LATIN SMALL LETTER V
	0x0077,	// w		LATIN SMALL LETTER W
	0x0078,	// x		LATIN SMALL LETTER X
	0x0079,	// y		LATIN SMALL LETTER Y
	0x007a,	// z		LATIN SMALL LETTER Z
	0x007b,	// {		LEFT CURLY BRACKET
	0x007c,	// |		VERTICAL LINE
	0x007d,	// }		RIGHT CURLY BRACKET
	0x007e,	// ~		TILDE
	0x007f,	// <DEL>	DELETE
	0x00c7,	// ??		LATIN CAPITAL LETTER C WITH CEDILLA
	0x00fc,	// ??		LATIN SMALL LETTER U WITH DIAERESIS
	0x00e9,	// ??		LATIN SMALL LETTER E WITH ACUTE
	0x00e2,	// ??		LATIN SMALL LETTER A WITH CIRCUMFLEX
	0x00e4,	// ??		LATIN SMALL LETTER A WITH DIAERESIS
	0x00e0,	// ??		LATIN SMALL LETTER A WITH GRAVE
	0x00e5,	// ??		LATIN SMALL LETTER A WITH RING ABOVE
	0x00e7,	// ??		LATIN SMALL LETTER C WITH CEDILLA
	0x00ea,	// ??		LATIN SMALL LETTER E WITH CIRCUMFLEX
	0x00eb,	// ??		LATIN SMALL LETTER E WITH DIAERESIS
	0x00e8,	// ??		LATIN SMALL LETTER E WITH GRAVE
	0x00ef,	// ??		LATIN SMALL LETTER I WITH DIAERESIS
	0x00ee,	// ??		LATIN SMALL LETTER I WITH CIRCUMFLEX
	0x00ec,	// ??		LATIN SMALL LETTER I WITH GRAVE
	0x00c4,	// ??		LATIN CAPITAL LETTER A WITH DIAERESIS
	0x00c5,	// ??		LATIN CAPITAL LETTER A WITH RING ABOVE
	0x00c9,	// ??		LATIN CAPITAL LETTER E WITH ACUTE
	0x00e6,	// ??		LATIN SMALL LETTER AE
	0x00c6,	// ??		LATIN CAPITAL LETTER AE
	0x00f4,	// ??		LATIN SMALL LETTER O WITH CIRCUMFLEX
	0x00f6,	// ??		LATIN SMALL LETTER O WITH DIAERESIS
	0x00f2,	// ??		LATIN SMALL LETTER O WITH GRAVE
	0x00fb,	// ??		LATIN SMALL LETTER U WITH CIRCUMFLEX
	0x00f9,	// ??		LATIN SMALL LETTER U WITH GRAVE
	0x00ff,	// ??		LATIN SMALL LETTER Y WITH DIAERESIS
	0x00d6,	// ??		LATIN CAPITAL LETTER O WITH DIAERESIS
	0x00dc,	// ??		LATIN CAPITAL LETTER U WITH DIAERESIS
	0x00a2,	// ??		CENT SIGN
	0x00a3,	// ??		POUND SIGN
	0x00a5,	// ??		YEN SIGN
	0x00df,	// ??		LATIN SMALL LETTER SHARP S
	0x0192,	// ??		LATIN SMALL LETTER F WITH HOOK
	0x00e1,	// ??		LATIN SMALL LETTER A WITH ACUTE
	0x00ed,	// ??		LATIN SMALL LETTER I WITH ACUTE
	0x00f3,	// ??		LATIN SMALL LETTER O WITH ACUTE
	0x00fa,	// ??		LATIN SMALL LETTER U WITH ACUTE
	0x00f1,	// ??		LATIN SMALL LETTER N WITH TILDE
	0x00d1,	// ??		LATIN CAPITAL LETTER N WITH TILDE
	0x00aa,	// ??		FEMININE ORDINAL INDICATOR
	0x00ba,	// ??		MASCULINE ORDINAL INDICATOR
	0x00bf,	// ??		INVERTED QUESTION MARK
	0x2310,	// ???		REVERSED NOT SIGN
	0x00ac,	// ??		NOT SIGN
	0x00bd,	// ??		VULGAR FRACTION ONE HALF
	0x00bc,	// ??		VULGAR FRACTION ONE QUARTER
	0x00a1,	// ??		INVERTED EXCLAMATION MARK
	0x00ab,	// ??		LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
	0x00bb,	// ??		RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
	0x00e3,	// ??		LATIN SMALL LETTER A WITH TILDE
	0x00f5,	// ??		LATIN SMALL LETTER O WITH TILDE
	0x00d8,	// ??		LATIN CAPITAL LETTER O WITH STROKE
	0x00f8,	// ??		LATIN SMALL LETTER O WITH STROKE
	0x0153,	// ??		LATIN SMALL LIGATURE OE
	0x0152,	// ??		LATIN CAPITAL LIGATURE OE
	0x00c0,	// ??		LATIN CAPITAL LETTER A WITH GRAVE
	0x00c3,	// ??		LATIN CAPITAL LETTER A WITH TILDE
	0x00d5,	// ??		LATIN CAPITAL LETTER O WITH TILDE
	0x00a8,	// ??		DIAERESIS
	0x00b4,	// ??		ACUTE ACCENT
	0x2020,	// ???		DAGGER
	0x00b6,	// ??		PILCROW SIGN
	0x00a9,	// ??		COPYRIGHT SIGN
	0x00ae,	// ??		REGISTERED SIGN
	0x2122,	// ???		TRADE MARK SIGN
	0x0133,	// ??		LATIN SMALL LIGATURE IJ
	0x0132,	// ??		LATIN CAPITAL LIGATURE IJ
	0x05d0,	// ??		HEBREW LETTER ALEF
	0x05d1,	// ??		HEBREW LETTER BET
	0x05d2,	// ??		HEBREW LETTER GIMEL
	0x05d3,	// ??		HEBREW LETTER DALET
	0x05d4,	// ??		HEBREW LETTER HE
	0x05d5,	// ??		HEBREW LETTER VAV
	0x05d6,	// ??		HEBREW LETTER ZAYIN
	0x05d7,	// ??		HEBREW LETTER HET
	0x05d8,	// ??		HEBREW LETTER TET
	0x05d9,	// ??		HEBREW LETTER YOD
	0x05db,	// ??		HEBREW LETTER KAF
	0x05dc,	// ??		HEBREW LETTER LAMED
	0x05de,	// ??		HEBREW LETTER MEM
	0x05e0,	// ??		HEBREW LETTER NUN
	0x05e1,	// ??		HEBREW LETTER SAMEKH
	0x05e2,	// ??		HEBREW LETTER AYIN
	0x05e4,	// ??		HEBREW LETTER PE
	0x05e6,	// ??		HEBREW LETTER TSADI
	0x05e7,	// ??		HEBREW LETTER QOF
	0x05e8,	// ??		HEBREW LETTER RESH
	0x05e9,	// ??		HEBREW LETTER SHIN
	0x05ea,	// ??		HEBREW LETTER TAV
	0x05df,	// ??		HEBREW LETTER FINAL NUN
	0x05da,	// ??		HEBREW LETTER FINAL KAF
	0x05dd,	// ??		HEBREW LETTER FINAL MEM
	0x05e3,	// ??		HEBREW LETTER FINAL PE
	0x05e5,	// ??		HEBREW LETTER FINAL TSADI
	0x00a7,	// ??		SECTION SIGN
	0x2227,	// ???		LOGICAL AND
	0x221e,	// ???		INFINITY
	0x03b1,	// ??		GREEK SMALL LETTER ALPHA
	0x03b2,	// ??		GREEK SMALL LETTER BETA
	0x0393,	// ??		GREEK CAPITAL LETTER GAMMA
	0x03c0,	// ??		GREEK SMALL LETTER PI
	0x03a3,	// ??		GREEK CAPITAL LETTER SIGMA
	0x03c3,	// ??		GREEK SMALL LETTER SIGMA
	0x00b5,	// ??		MICRO SIGN
	0x03c4,	// ??		GREEK SMALL LETTER TAU
	0x03a6,	// ??		GREEK CAPITAL LETTER PHI
	0x0398,	// ??		GREEK CAPITAL LETTER THETA
	0x03a9,	// ??		GREEK CAPITAL LETTER OMEGA
	0x03b4,	// ??		GREEK SMALL LETTER DELTA
	0x222e,	// ???		CONTOUR INTEGRAL
	0x03c6,	// ??		GREEK SMALL LETTER PHI
	0x2208,	// ???		ELEMENT OF
	0x2229,	// ???		INTERSECTION
	0x2261,	// ???		IDENTICAL TO
	0x00b1,	// ??		PLUS-MINUS SIGN
	0x2265,	// ???		GREATER-THAN OR EQUAL TO
	0x2264,	// ???		LESS-THAN OR EQUAL TO
	0x2320,	// ???		TOP HALF INTEGRAL
	0x2321,	// ???		BOTTOM HALF INTEGRAL
	0x00f7,	// ??		DIVISION SIGN
	0x2248,	// ???		ALMOST EQUAL TO
	0x00b0,	// ??		DEGREE SIGN
	0x2219,	// ???		BULLET OPERATOR
	0x00b7,	// ??		MIDDLE DOT
	0x221a,	// ???		SQUARE ROOT
	0x207f,	// ???		SUPERSCRIPT LATIN SMALL LETTER N
	0x00b2,	// ??		SUPERSCRIPT TWO
	0x00b3,	// ??		SUPERSCRIPT THREE
	0x00af	// ??		MACRON
};

unicode_t charset_atari_st_to_utf32(uint8_t c, void *arg)
{
	return atari_st_charset[c];
}

uint8_t utf32_to_charset_atari_st(unicode_t u, void *arg)
{
	for (int c = 0; c < ARRAY_SIZE(atari_st_charset); c++)
		if (u == atari_st_charset[c])
			return c;

	return '?';
}

bool utf8_valid_in_atari_st(const uint8_t *u, size_t length)
{
	return utf8_valid_in_charset_string(u, length,
			charset_atari_st_to_utf32,
			utf32_to_charset_atari_st, NULL);
}
