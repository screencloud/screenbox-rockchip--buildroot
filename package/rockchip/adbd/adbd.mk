# Rockchip's adbd porting for Linux
# Author : Cody Xie <cody.xie@rock-chips.com>

ADBD_SITE = $(call qstrip, ssh://git@10.10.10.78:2222/argus/externals/adb.git)
ADBD_SITE_METHOD = git
ADBD_VERSION = 0d8ec12
ADBD_SOURCE = adbd-${ADBD_VERSION}.tar.gz
ADBD_FROM_GIT = y

ADBD_INIT_SCRIPT=package/rockchip/adbd/S30adbd

define ADBD_INSTALL_INIT_SCRIPT
$(INSTALL) -D -m 0755 ${ADBD_INIT_SCRIPT}\
		$(TARGET_DIR)/etc/init.d/
endef
ADBD_POST_INSTALL_TARGET_HOOKS+=ADBD_INSTALL_INIT_SCRIPT

$(eval $(cmake-package))
#$(eval $(generic-package))
