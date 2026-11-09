PKG_NAME=virtwip
PKG_URL=https://github.com/lwip-tcpip/lwip.git
# lwIP v2.2.1
PKG_VERSION=77dcd25a72509eb83f72b033d219b1d40cd8eb95
PKG_LICENSE=BSD-3-Clause

PKG_SOURCE_LOCAL = /home/leo/projects/lwip/

include $(RIOTBASE)/pkg/pkg.mk

# The lwIP configuration is not passed as -D flags: Makefile.include (parsed
# by the application build) generates virtwip_config.h from Makefile.conf,
# which include_shared/lwipopts.h includes on both sides.
VIRTWIP_CONFIG_DIR ?= $(BINDIR)/virtwip_config
VIRTWIP_CONFIG_H ?= $(VIRTWIP_CONFIG_DIR)/virtwip_config.h

# VIRTWIP_VM marks code compiled into the VM image (see lwipopts.h)
VIRTWIP_VM_FLAGS = -DVIRTWIP_VM=1

VIRTWIP_INCLUDES ?= -I$(PKG_SOURCE_DIR)/src/include
VIRTWIP_INCLUDES += -I$(VIRTWIP_CONFIG_DIR)
VIRTWIP_INCLUDES += -I$(CURDIR)/include_vm
VIRTWIP_INCLUDES += -I$(CURDIR)/include_shared
VIRTWIP_INCLUDES += -I$(RIOTBASE)/pkg/femto-container/include_vm

VIRTWIP_CONTRIB_SRC ?= $(wildcard $(RIOTPKG)/virtwip/contrib_vm/*.c)
VIRTWIP_CONTRIB_SRC += $(wildcard $(RIOTPKG)/virtwip/contrib_vm/icd/*.c)

VIRTWIP_CONTRIB_OBJS := $(subst $(RIOTPKG)/virtwip/contrib_vm, $(PKG_BUILD_DIR), $(VIRTWIP_CONTRIB_SRC))
VIRTWIP_CONTRIB_OBJS := $(VIRTWIP_CONTRIB_OBJS:.c=.ll)

ifneq (,$(filter virtwip_api, $(USEMODULE)))
  VIRTWIP_PKG_SRC += $(wildcard $(PKG_SOURCE_DIR)/src/api/*.c)
endif
ifneq (,$(filter virtwip_core, $(USEMODULE)))
  VIRTWIP_PKG_SRC += $(wildcard $(PKG_SOURCE_DIR)/src/core/*.c)
endif
ifneq (,$(filter virtwip_ipv4, $(USEMODULE)))
  VIRTWIP_PKG_SRC += $(wildcard $(PKG_SOURCE_DIR)/src/core/ipv4/*.c)
endif
ifneq (,$(filter virtwip_ipv6, $(USEMODULE)))
  VIRTWIP_PKG_SRC += $(wildcard $(PKG_SOURCE_DIR)/src/core/ipv6/*.c)
endif
ifneq (,$(filter virtwip_netif, $(USEMODULE)))
  VIRTWIP_PKG_SRC += $(wildcard $(PKG_SOURCE_DIR)/src/netif/*.c)
endif

# slipif requires an sio_* port implementation which does not exist for
# RIOT (SLIP goes through the netdev glue instead): exclude it and guard
# the ICD dispatch cases with VIRTWIP_ENABLE_SLIPIF.
VIRTWIP_PKG_SRC := $(filter-out $(PKG_SOURCE_DIR)/src/netif/slipif.c,$(VIRTWIP_PKG_SRC))

VIRTWIP_PKG_OBJS := $(subst $(PKG_SOURCE_DIR), $(PKG_BUILD_DIR), $(VIRTWIP_PKG_SRC))
VIRTWIP_PKG_OBJS := $(VIRTWIP_PKG_OBJS:.c=.ll)

EMBPF_GENEMBPF=$(PKGDIRBASE)/femto-container/tools/gen_embpf.py

CLANG ?= clang
LLC ?= llc
LLVM_LINK ?= llvm-link-21

.SECONDEXPANSION:

.PHONY: all

# The image as C array for the host build (contrib/vm.c). femto reads .text,
# .rodata and the function table in place, so the array must be 8-byte
# aligned; RIOT's BLOBS mechanism (xxd -i) gives no alignment guarantee.
VIRTWIP_IMAGE_DIR ?= $(BINDIR)/virtwip_image
XXD ?= xxd

all: $(VIRTWIP_IMAGE_DIR)/virtwip_image.h

$(VIRTWIP_IMAGE_DIR)/virtwip_image.h: $(PKG_BUILD_DIR)/lwip.bin | $$(@D)/
	cd $(PKG_BUILD_DIR) && $(XXD) -i lwip.bin | sed \
		-e 's/^unsigned char /const unsigned char __attribute__((aligned(8))) /' \
		-e 's/^unsigned int /const unsigned int /' > $@

%/:
	@mkdir -p $@

dump: $(PKG_BUILD_DIR)/lwip.bin
	$(EMBPF_GENEMBPF) -d dump $@

$(PKG_BUILD_DIR)/lwip.bin: $(PKG_BUILD_DIR)/lwip.o
	$(EMBPF_GENEMBPF) -d generate $< $@

# -mcpu=v3 (ALU32 + JMP32). Do not use -mcpu=v2 -mattr=+alu32: LLVM 21 then
# zero-extends 32-bit values (w2 = w0) and compares them against
# sign-extended 64-bit immediates (if r2 == -1), which is never true.
$(PKG_BUILD_DIR)/lwip.o: $(VIRTWIP_PKG_OBJS) $(VIRTWIP_CONTRIB_OBJS)
	$(LLVM_LINK) $^ -o $(PKG_BUILD_DIR)/combined.ll
	$(LLC) $(PKG_BUILD_DIR)/combined.ll -march=bpf -mcpu=v3 -mattr=+alu32 \
		-filetype=obj -stack-size-section -o $@

$(PKG_BUILD_DIR)/%.ll: $(PKG_SOURCE_DIR)/%.c $(VIRTWIP_CONFIG_H) | $$(@D)/
	$(CLANG) -nostdinc -isystem `$(CLANG) -print-file-name=include` \
		-Wno-unused-value -Wno-pointer-sign -g3\
		-Wno-compare-distinct-pointer-types \
		-Wno-gnu-variable-sized-type-not-at-end \
		-Wno-address-of-packed-member -Wno-tautological-compare \
		-Wno-unknown-warning-option -fstack-size-section \
		-fno-builtin \
		$(VIRTWIP_VM_FLAGS) \
		$(VIRTWIP_INCLUDES) \
		-Os -emit-llvm -c $< -o $@

$(PKG_BUILD_DIR)/%.ll: $(RIOTPKG)/virtwip/contrib_vm/%.c $(VIRTWIP_CONFIG_H) | $$(@D)/
	$(CLANG) -nostdinc -isystem `$(CLANG) -print-file-name=include` \
		-Wno-unused-value -Wno-pointer-sign -g3\
		-Wno-compare-distinct-pointer-types \
		-Wno-gnu-variable-sized-type-not-at-end \
		-Wno-address-of-packed-member -Wno-tautological-compare \
		-Wno-unknown-warning-option -fstack-size-section \
		-fno-builtin \
		$(VIRTWIP_VM_FLAGS) \
		$(VIRTWIP_INCLUDES) \
		-Os -emit-llvm -c $< -o $@



