# Rockchip's liblog porting from Android
# Author : Cody Xie <cody.xie@rock-chips.com>

ifeq ($(BR2_PACKAGE_ALEXACLIENTSDK),y)
ALEXACLIENTSDK_SITE = $(TOPDIR)/../external/alexaClientSDK
ALEXACLIENTSDK_SITE_METHOD = local
ALEXACLIENTSDK_INSTALL_STAGING = YES

ALEXACLIENTSDK_CONF_OPTS +=\
						   CMAKE_CURRENT_SOURCE_DIR= source \
						   -DLIBRARY_OUTPUT_PATH=$(TOPDIR)/output/target/usr/lib/ \
						   -DEXECUTABLE_OUTPUT_PATH=$(TOPDIR)/output/target/usr/bin \
						   -DCMAKE_BUILD_TYPE=DEBUG \
						   -DKITTAI_KEY_WORD_DETECTOR=ON \
						   -DKITTAI_KEY_WORD_DETECTOR_LIB_PATH=$(TOPDIR)/output/build/alexaClientSDK/source/snowboy/lib/rpi/libsnowboy-detect.a \
						   -DKITTAI_KEY_WORD_DETECTOR_INCLUDE_DIR=$(TOPDIR)/output/build/alexaClientSDK/source/snowboy/include \
						   -DGSTREAMER_MEDIA_PLAYER=ON
$(eval $(cmake-package))
endif


