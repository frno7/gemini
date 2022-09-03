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

/* Reverse order for proper BE_STORAGE structure bit field order. */
#define GEM_OBJECT_FLAG(f)						\
	f(11, submenu,    SUBMENU)					\
	f(10, fl3dact,    FL3DACT)					\
	f( 9, fl3dind,    FL3DIND)					\
	f( 8, indirect,   INDIRECT)					\
	f( 7, hidetree,   HIDETREE)					\
	f( 6, touchexit,  TOUCHEXIT)					\
	f( 5, lastob,     LASTOB)					\
	f( 4, rbutton,    RBUTTON)					\
	f( 3, editable,   EDITABLE)					\
	f( 2, exit,       EXIT)						\
	f( 1, default_,   DEFAULT)					\
	f( 0, selectable, SELECTABLE)

/* Reverse order for proper BE_STORAGE structure bit field order. */
#define GEM_OBJECT_STATE(s)						\
	s(5, shadowed, SHADOWED)					\
	s(4, outlined, OUTLINED)					\
	s(3, disabled, DISABLED)					\
	s(2, checked,  CHECKED)						\
	s(1, crossed,  CROSSED)						\
	s(0, selected, SELECTED)

#define GEM_TEDINFO_FONT(f)						\
	f(0, gdos_prop, GDOS_PROP)					\
	f(1, gdos_mono, GDOS_MONO)					\
	f(2, gdos_bitm, GDOS_BITM)					\
	f(3, large,     LARGE)						\
	f(5, small,     SMALL)

#endif /* _GEM_OBJECT_H */
