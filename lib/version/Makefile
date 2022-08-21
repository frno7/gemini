# SPDX-License-Identifier: GPL-2.0

GEMINI_VERSION := $(shell script/version)
GEMINI_VERSION_MINOR := $(shell echo '$(GEMINI_VERSION)' | sed 's/-.*$$//')
GEMINI_VERSION_MAJOR := $(shell echo '$(GEMINI_VERSION)' | sed 's/\..*$$//')

export GEMINI_VERSION_MINOR GEMINI_VERSION_MAJOR

VERSION_SRC = lib/version/version.c

VERSION_OBJ = $(VERSION_SRC:%.c=%.o)

ALL_OBJ += $(VERSION_OBJ)

.PHONY: $(shell script/version --check $(VERSION_SRC))
$(VERSION_SRC):
	$(QUIET_GEN)script/version $@

OTHER_CLEAN += $(VERSION_SRC)
