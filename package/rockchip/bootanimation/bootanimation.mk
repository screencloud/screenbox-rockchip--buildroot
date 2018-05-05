# Rockchip's MPP(Multimedia Processing Platform)
BOOTANIMATION_SITE = $(TOPDIR)/package/rockchip/bootanimation/src
BOOTANIMATION_VERSION = release
BOOTANIMATION_SITE_METHOD = local

BOOTANIMATION_CONF_DEPENDENCIES += libdrm

$(eval $(cmake-package))
