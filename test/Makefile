# SPDX-License-Identifier: GPL-2.0

TEST_RSC =								\
	test/tos206se.rsc						\
	test/tos206uk.rsc

.PHONY: $(TEST_RSC)
$(TEST_RSC): $(TOOL)
	$(QUIET_CHECK)$(TOOL_RSC) --identify $@

.PHONY: test
test: $(TEST_RSC)
