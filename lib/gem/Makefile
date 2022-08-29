# SPDX-License-Identifier: GPL-2.0

GEM_SRC =								\
	lib/gem/fnt.c							\
	lib/gem/rsc.c

GEMLIB_SRC = $(GEM_SRC)

GEMLIB_OBJ = $(GEMLIB_SRC:%.c=%.o)

GEMLIB = lib/gem/gemlib.a

ALL_OBJ += $(GEMLIB_OBJ)
OTHER_CLEAN += $(GEMLIB)

$(GEMLIB): $(GEMLIB_OBJ) $(UNICODE_OBJ) $(INTERNAL_OBJ) $(VERSION_OBJ)
	$(QUIET_AR)$(AR) rcs $@ $^

.PHONY: gemlib
gemlib: $(GEMLIB)
