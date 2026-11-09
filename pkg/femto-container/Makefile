PKG_NAME    = femto-container
PKG_VERSION = 9dfd075e29fe7c6a4e34b0e81a7842be1681771b
PKG_URL     = git@github.com:LeonardHerbst/Femto-Container.git
PKG_LICENSE = apache

PKG_SOURCE_LOCAL = /home/leo/projects/benchmark-language-runtimes/external/femto-containers-src

include $(RIOTBASE)/pkg/pkg.mk

all:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/src -f $(PKG_DIR)/Makefile.femto MODULE=$(PKG_NAME)
