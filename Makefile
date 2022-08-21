# SPDX-License-Identifier: GPL-2.0

CFLAGS = -g

ifeq (1,$(S))
S_CFLAGS = -fsanitize=address -fsanitize=leak -fsanitize=undefined	\
	  -fsanitize-address-use-after-scope -fstack-check
endif

DEP_CFLAGS = -Wp,-MD,$(@D)/$(@F).d -MT $(@D)/$(@F)
BASIC_CFLAGS = -O2 -Wall -D_GNU_SOURCE $(DEP_CFLAGS)
ALL_CFLAGS = -Iinclude $(BASIC_CFLAGS) $(S_CFLAGS) $(CFLAGS)

.PHONY: all
all:

include lib/Makefile
include tool/Makefile
include test/Makefile

all: gemlib tool

ALL_DEP = $(sort $(ALL_OBJ:%=%.d))

$(ALL_OBJ): %.o: %.c
	$(QUIET_CC)$(CC) $(ALL_CFLAGS) -c -o $@ $<

.PHONY: version
version:
	@script/version

.PHONY: gtags
gtags:
	$(QUIET_GEN)gtags
OTHER_CLEAN += GPATH GRTAGS GTAGS

.PHONY: clean
clean:
	$(QUIET_RM)$(RM) $(ALL_OBJ) $(ALL_DEP) $(OTHER_CLEAN)

V             = @
Q             = $(V:1=)
QUIET_AR      = $(Q:@=@echo    '  AR      '$@;)
QUIET_AS      = $(Q:@=@echo    '  AS      '$@;)
QUIET_CC      = $(Q:@=@echo    '  CC      '$@;)
QUIET_GEN     = $(Q:@=@echo    '  GEN     '$@;)
QUIET_LD      = $(Q:@=@echo    '  LD      '$@;)
QUIET_RM      = $(Q:@=@echo    '  RM      '$@;)
QUIET_CHECK   = $(Q:@=@echo    '  CHECK   '$@;)
QUIET_TEST    = $(Q:@=@echo    '  TEST    '$@;)

$(eval -include $(ALL_DEP))
