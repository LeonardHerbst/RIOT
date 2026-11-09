PKG_NAME    = femto-container
PKG_VERSION = 7b29e5b2e3589c9f4320bfd70649f53b6b81d05a
PKG_URL     = git@github.com:LeonardHerbst/Femto-Container.git
PKG_LICENSE = apache

PKG_SOURCE_LOCAL = /home/leo/projects/benchmark-language-runtimes/external/femto-containers-src

include $(RIOTBASE)/pkg/pkg.mk

all:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/src -f $(PKG_DIR)/Makefile.femto MODULE=$(PKG_NAME)
