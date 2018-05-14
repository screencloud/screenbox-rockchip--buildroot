################################################################################
#
# ueventd
#
################################################################################

UEVENTD_LICENSE_FILES = NOTICE
UEVENTD_LICENSE = Apache V2.0

define UEVENTD_TARGET_CFLAGS
	$(TARGET_CFLAGS) \
	-D_GNU_SOURCE \
	-I$(STAGING_DIR)/usr/include \
	-I$(TOPDIR)/package/rockchip/ueventd/include
endef

define UEVENTD_BUILD_CMDS
	$(TARGET_CC) $(UEVENTD_TARGET_CFLAGS) $(TARGET_LDFLAGS) \
		package/rockchip/ueventd/devices.c \
		package/rockchip/ueventd/util.c \
		package/rockchip/ueventd/ueventd.c \
		package/rockchip/ueventd/ueventd_parser.c \
		package/rockchip/ueventd/parser.c \
		-o $(@D)/ueventd
endef

define UEVENTD_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/ueventd $(TARGET_DIR)/usr/bin/ueventd
endef

$(eval $(generic-package))
