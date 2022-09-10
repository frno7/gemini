# SPDX-License-Identifier: GPL-2.0

STORAGE_SRC = lib/gem/storage.c

GEM_SRC =								\
	lib/gem/aes.c							\
	lib/gem/aes-area.c						\
	lib/gem/aes-filter.c						\
	lib/gem/aes-pixel.c						\
	lib/gem/aes-rsc.c						\
	lib/gem/aes-shape.c						\
	lib/gem/fnt.c							\
	lib/gem/rsc.c							\
	lib/gem/rsc-map.c						\
	lib/gem/vdi.c

GEMLIB_SRC = $(GEM_SRC) $(STORAGE_SRC)

GEMLIB_OBJ = $(GEMLIB_SRC:%.c=%.o)

STORAGE =								\
	lib/gem/system6x6.fnt						\
	lib/gem/system8x8.fnt						\
	lib/gem/system8x16.fnt

GEMLIB = lib/gem/gemlib.a

ALL_OBJ += $(GEMLIB_OBJ)
OTHER_CLEAN += $(GEMLIB) $(STORAGE_SRC)

STORAGE_SCRIPT = script/storage

$(STORAGE_SRC): $(STORAGE) $(STORAGE_SCRIPT)
	$(QUIET_GEN)$(STORAGE_SCRIPT) -o $@ $(STORAGE)

$(GEMLIB): $(GEMLIB_OBJ) $(UNICODE_OBJ) $(INTERNAL_OBJ) $(VERSION_OBJ)
	$(QUIET_AR)$(AR) rcs $@ $^

.PHONY: gemlib
gemlib: $(GEMLIB)
