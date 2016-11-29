# Rockchip's MPP(Multimedia Processing Platform)
# Author : Cody Xie <cody.xie@rock-chips.com>

MPP_VERSION = afb088e0
MPP_SITE =  $(call github,HermanChen,mpp,$(MPP_VERSION))
MPP_INSTALL_STAGING = YES
MPP_SOURCE = mpp-${MPP_VERSION}.tar.gz
MPP_FROM_GIT = y

MPP_CONF_OPTS = -DCMAKE_C_FLAGS="${TARGET_CFLAGS} -fPIC -DARMLINUX"
MPP_CONF_OPTS += -DCMAKE_CXX_FLAGS="${TARGET_CXXFLAGS} -fPIC -DARMLINUX"

MPP_CONF_OPTS += -DCMAKE_BUILD_TYPE=Release
MPP_CONF_OPTS += -DCMAKE_RKPLATFORM_ENABLE=ON
MPP_CONF_OPTS += -G "Unix Makefiles"

MPP_CONF_OPTS += -DLIB_INSTALL_DIR=/usr/lib
MPP_CONF_OPTS += -DBIN_INSTALL_DIR=/usr/bin
MPP_CONF_OPTS += -DTEST_INSTALL_DIR=/usr/sbin
MPP_CONF_OPTS += -DINC_INSTALL_DIR=/usr/include/mpp

ifneq (${BR2_ROCKCHIP_MPP_TEST}, y)
define MPP_RM_TARGET_TEST
rm -rf $(TARGET_DIR)/usr/test
endef
MPP_POST_INSTALL_TARGET_HOOKS += MPP_RM_TARGET_TEST
endif

$(eval $(cmake-package))
