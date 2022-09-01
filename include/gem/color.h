// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#ifndef _GEM_COLOR_H
#define _GEM_COLOR_H

#define GEM_PALETTE_COLOR(c)						\
	c( 0, 1000, 1000, 1000, white,    WHITE,    "white")		\
	c( 1,    0,    0,    0, black,    BLACK,    "black")		\
	c( 2, 1000,    0,    0, red,      RED,      "red")		\
	c( 3,    0, 1000,    0, green,    GREEN,    "green")		\
	c( 4,    0,    0, 1000, blue,     BLUE,     "blue")		\
	c( 5,    0, 1000, 1000, cyan,     CYAN,     "cyan")		\
	c( 6, 1000, 1000,    0, yellow,   YELLOW,   "yellow")		\
	c( 7, 1000,    0, 1000, magenta,  MAGENTA,  "magenta")		\
	c( 8,  667,  667,  667, lwhite,   LWHITE,   "light gray")	\
	c( 9,  400,  400,  400, lblack,   LBLACK,   "dark gray")	\
	c(10, 1000,  400,  400, lred,     LRED,     "light red")	\
	c(11,  400, 1000,  400, lgreen,   LGREEN,   "light green")	\
	c(12,  400,  400, 1000, lblue,    LBLUE,    "light blue")	\
	c(13,  400, 1000, 1000, lcyan,    LCYAN,    "light cyan")	\
	c(14, 1000, 1000,  400, lyellow,  LYELLOW,  "light yellow")	\
	c(15, 1000,  400, 1000, lmagenta, LMAGENTA, "light magenta")

#endif /* _GEM_COLOR_H */
