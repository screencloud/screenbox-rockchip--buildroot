# Rockchip's softap demo
# Author : Jacky <jorge.ge@rock-chips.com>

ifeq ($(BR2_PACKAGE_SOFTAP), y)
SOFTAP_SITE = $(TOPDIR)/../external/softapDemo
SOFTAP_SITE_METHOD = local
SOFTAP_INSTALL_STAGING = YES
SOFTAP_CONF_OPTS +=\
    -DLIBRARY_OUTPUT_PATH=$(TOPDIR)/board/rockchip/rk3036/fs-overlay/usr/lib \
    -DEXECUTABLE_OUTPUT_PATH=$(TOPDIR)/board/rockchip/rk3036/fs-overlay/usr/bin

$(eval $(cmake-package))
endif


