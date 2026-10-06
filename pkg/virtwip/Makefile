PKG_NAME=virtwip
PKG_URL=https://github.com/lwip-tcpip/lwip.git
# lwIP v2.2.1
PKG_VERSION=77dcd25a72509eb83f72b033d219b1d40cd8eb95
PKG_LICENSE=BSD-3-Clause

include $(RIOTBASE)/pkg/pkg.mk

VIRTWIP_MODULES         = virtwip_api virtwip_app_lwiperf virtwip_core \
                            virtwip_ipv4 virtwip_ipv6 virtwip_netif \
                            virtwip_netif_ppp virtwip_polarssl
VIRTWIP_USEMODULE       = $(filter $(VIRTWIP_MODULES),$(USEMODULE))
VIRTWIP_MODULE_MAKEFILE = $(RIOTBASE)/Makefile.base

.PHONY: $(VIRTWIP_MODULES)

CFLAGS += -Wno-address
# When compiled e.g. with `CFLAGS += -DIP6_DEBUG=LWIP_DBG_ON`, debug output
# does not use the correct format specifiers and compilation fails. (All
# instances were differently named but same sized integer types, e.g. %lx
# for printing int when sizeof(int) == sizeof(long int).)
CFLAGS += -Wno-format

make_module = +$(QQ)"$(MAKE)" -C $(2) -f $(VIRTWIP_MODULE_MAKEFILE) MODULE=$(1)

all: virtwip

virtwip: $(VIRTWIP_USEMODULE)
	$(call make_module,$@,$(PKG_SOURCE_DIR))

virtwip_api:
	$(call make_module,$@,$(PKG_SOURCE_DIR)/src/api)

virtwip_app_lwiperf:
	$(call make_module,$@,$(PKG_SOURCE_DIR)/src/apps/lwiperf)

virtwip_core:
	$(call make_module,$@,$(PKG_SOURCE_DIR)/src/core)

virtwip_ipv4:
	$(call make_module,$@,$(PKG_SOURCE_DIR)/src/core/ipv4)

virtwip_ipv6:
	$(call make_module,$@,$(PKG_SOURCE_DIR)/src/core/ipv6)

virtwip_netif:
	$(call make_module,$@,$(PKG_SOURCE_DIR)/src/netif)

virtwip_netif_ppp:
	$(call make_module,$@,$(PKG_SOURCE_DIR)/src/netif/ppp)

virtwip_polarssl:
	$(call make_module,$@,$(PKG_SOURCE_DIR)/src/netif/ppp/polarssl)
