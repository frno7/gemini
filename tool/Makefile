# SPDX-License-Identifier: GPL-2.0

TOOL_SRC =								\
	tool/rsc.c

TOOL_OBJ = $(TOOL_SRC:%.c=%.o)
TOOL = $(TOOL_SRC:%.c=%)

TOOL_RSC = tool/rsc

ALL_OBJ += $(TOOL_OBJ)

$(TOOL): $(GEMLIB)

$(TOOL): %: %.o $(INTERNAL_OBJ)
	$(QUIET_LD)$(CC) $(ALL_CFLAGS) -o $@ $^

OTHER_CLEAN += $(TOOL)

.PHONY: tool
tool: $(TOOL)
