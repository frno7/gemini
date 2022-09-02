# SPDX-License-Identifier: GPL-2.0

TEST_RSC =								\
	test/tos206se.rsc						\
	test/tos206uk.rsc

TEST = $(TEST_RSC:%.rsc=%)

.PHONY: $(TEST)
$(TEST): $(TOOL)
	$(QUIET_CHECK)$(TOOL_RSC) --identify $@.rsc

.PHONY: test
test: $(TEST)
