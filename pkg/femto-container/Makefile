PKG_NAME    = femto-container
PKG_VERSION = ac3559cf89dc12beae51f6151b2e557304cfdfb0
PKG_URL     = git@github.com:LeonardHerbst/Femto-Container.git
PKG_LICENSE = apache

include $(RIOTBASE)/pkg/pkg.mk

all:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/src -f $(PKG_DIR)/Makefile.femto MODULE=$(PKG_NAME)
