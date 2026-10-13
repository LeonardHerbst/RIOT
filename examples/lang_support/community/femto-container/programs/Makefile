RIOTBASE ?= $(CURDIR)/../../RIOT
EMBPF_GENEMBPF = $(RIOTBASE)/dist/tools/embpf/gen_embpf.py

HEAP_SIZE ?= 0

# export EXTRA_CFLAGS += -Os -emit-llvm -DSCALE_FACTOR=$(SCALE_FACTOR)
export EXTRA_CFLAGS += -fno-builtin-memcpy -fno-builtin-memset -Os -fstack-size-section

# CLANG := broken
include $(RIOTBASE)/makefiles/rbpf.inc.mk

.PHONY: all test

# EMBPF_SOURCES += -I$(CURDIR)/helpers.h

DUMP_FILE ?= ../program

dump:
	$(EMBPF_GENEMBPF) dump $(DUMP_FILE).bin
