// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (C) 2022 Fredrik Noring
 */

#ifndef _GEM_OBJECT_H
#define _GEM_OBJECT_H

#define GEM_OBJECT_G_TYPE(t)						\
	t(20, box,      BOX,      box)					\
	t(21, text,     TEXT,     tedinfo)				\
	t(22, boxtext,  BOXTEXT,  tedinfo)				\
	t(23, image,    IMAGE,    bitblk)				\
	t(24, progdef,  PROGDEF,  applblk)				\
	t(25, ibox,     IBOX,     box)					\
	t(26, button,   BUTTON,   string)				\
	t(27, boxchar,  BOXCHAR,  box)					\
	t(28, string,   STRING,   string)				\
	t(29, ftext,    FTEXT,    tedinfo)				\
	t(30, fboxtext, FBOXTEXT, tedinfo)				\
	t(31, icon,     ICON,     iconblk)				\
	t(32, title,    TITLE,    string)				\
	t(33, cicon,    CICON,    ciconblk)

#endif /* _GEM_OBJECT_H */
