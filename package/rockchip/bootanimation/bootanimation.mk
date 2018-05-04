# Rockchip's MPP(Multimedia Processing Platform)
BOOTANIMATION_SITE = $(TOPDIR)/package/rockchip/bootanimation/src
BOOTANIMATION_VERSION = release
BOOTANIMATION_SITE_METHOD = local

BOOTANIMATION_CONF_DEPENDENCIES += libdrm

define BOOTANIMATION_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/S30BootAnimation $(TARGET_DIR)/etc/init.d/
	$(INSTALL) -D -m 0755 $(@D)/start_launcher.sh $(TARGET_DIR)/usr/bin/
endef
$(eval $(cmake-package))
