PKG_NAME    = femto-container
PKG_VERSION = 6dae6320a3bcf09c94d87d04db82fb1368a14765
PKG_URL     = git@github.com:LeonardHerbst/Femto-Container.git
PKG_LICENSE = apache

PKG_SOURCE_LOCAL = /home/leo/projects/benchmark-language-runtimes/external/femto-containers-src

include $(RIOTBASE)/pkg/pkg.mk

all:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/src -f $(PKG_DIR)/Makefile.femto MODULE=$(PKG_NAME)
