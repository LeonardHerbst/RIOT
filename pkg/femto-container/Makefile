PKG_NAME    = femto-container
PKG_VERSION = 3cd025a6798ba42b9357f2cca08e8b760cbce7f9
PKG_URL     = git@github.com:LeonardHerbst/Femto-Container.git
PKG_LICENSE = apache

PKG_SOURCE_LOCAL = /home/leo/projects/benchmark-language-runtimes/external/femto-containers-src

include $(RIOTBASE)/pkg/pkg.mk

all:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/src -f $(PKG_DIR)/Makefile.femto MODULE=$(PKG_NAME)
