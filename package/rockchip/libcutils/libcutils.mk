# Rockchip's libcutils porting from Android
# Author : Cody Xie <cody.xie@rock-chips.com>

LIBCUTILS_SITE = $(call qstrip, ssh://git@10.10.10.78:2222/argus/externals/libcutils.git)
LIBCUTILS_SITE_METHOD = git
LIBCUTILS_VERSION = 2c61c38
LIBCUTILS_INSTALL_STAGING = YES
LIBCUTILS_SOURCE = libcutils-${LIBCUTILS_VERSION}.tar.gz
LIBCUTILS_FROM_GIT = y

$(eval $(cmake-package))
