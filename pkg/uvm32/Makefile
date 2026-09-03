PKG_NAME=uvm32
PKG_URL=https://github.com/ringtailsoftware/uvm32.git
PKG_VERSION=6055e7f9df70f6b381dcd68924ec4d1c88fc48a0
PKG_LICENSE=MIT

include $(RIOTBASE)/pkg/pkg.mk

UVM32_MEMORY_SIZE ?= 512

CFLAGS += -Wall -Wno-sign-compare -O2 -DUVM32_ERROR_STRINGS \
					-DUVM32_MEMORY_SIZE=$(UVM32_MEMORY_SIZE)

# gcc \
# 	$(CFLAGS) \
# 	-DUVM32_ERROR_STRINGS \
# 	-DUVM32_MEMORY_SIZE=512 \
# 	-O2 -I${TOPDIR}/uvm32 -I${TOPDIR}/common \
# 	-o host-mini ${TOPDIR}/uvm32/uvm32.c host-mini.c

all: $(PKG_NAME).mk
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR) -f $(CURDIR)/$(PKG_NAME).mk
