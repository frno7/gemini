# SPDX-License-Identifier: GPL-2.0

TEST_RSC =								\
	test/tos206se.rsc						\
	test/tos206uk.rsc

TEST = $(TEST_RSC:%.rsc=%)

.PHONY: $(TEST)
$(TEST): $(TOOL)
	$(QUIET_CHECK)$(TOOL_RSC) --identify $@.rsc
	@$(TOOL_RSC) --draw -o /dev/null $@.rsc

.PHONY: test
test: $(TEST)

TEST_TIFF = $(TEST_RSC:%.rsc=%.tiff)

$(TEST_TIFF): $(TOOL_RSC)

$(TEST_TIFF): %.tiff: %.rsc
	$(QUIET_GEN)$(TOOL_RSC) -o $@ --draw $<

.PHONY: tiff
tiff: $(TEST_TIFF)

OTHER_CLEAN += $(TEST_TIFF)
