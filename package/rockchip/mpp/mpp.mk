# Rockchip's MPP(Multimedia Processing Platform)

MPP_SITE = "git://github.com/rockchip-linux/mpp.git"
MPP_VERSION = for_linux
MPP_CONF_OPTS = "-DRKPLATFORM=ON"

MPP_INSTALL_STAGING = YES

$(eval $(cmake-package))
