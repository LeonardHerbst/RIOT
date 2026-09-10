PKG_NAME=uvm32
PKG_URL=https://github.com/ringtailsoftware/uvm32.git
PKG_VERSION=6055e7f9df70f6b381dcd68924ec4d1c88fc48a0
PKG_LICENSE=MIT

include $(RIOTBASE)/pkg/pkg.mk

CFLAGS += -Wall -Wno-sign-compare -O0

all: $(PKG_NAME).mk
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR) -f $(CURDIR)/$(PKG_NAME).mk
