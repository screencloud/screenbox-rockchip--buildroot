# Rockchip's liblog porting from Android
# Author : Cody Xie <cody.xie@rock-chips.com>

ifeq ($(BR2_PACKAGE_ALEXACLIENTSDK),y)
ALEXACLIENTSDK_SITE = $(TOPDIR)/../external/alexaClientSDK
ALEXACLIENTSDK_SITE_METHOD = local
ALEXACLIENTSDK_INSTALL_STAGING = YES
ALEXACLIENTSDK_DEPENDENCIES = libcurl libnghttp2  gst1-plugins-base gst1-plugins-ugly portaudio
ALEXACLIENTSDK_CONF_OPTS +=\
						   CMAKE_CURRENT_SOURCE_DIR= source \
						   -DLIBRARY_OUTPUT_PATH=$(TOPDIR)/output/target/usr/lib/ \
						   -DEXECUTABLE_OUTPUT_PATH=$(TOPDIR)/output/target/usr/bin \
						   -DCMAKE_BUILD_TYPE=DEBUG \
						   -DGSTREAMER_MEDIA_PLAYER=ON

ALEXACLIENTSDK_CONF_OPTS +=\
						   -DSENSORY_KEY_WORD_DETECTOR=ON \
						   -DSENSORY_KEY_WORD_DETECTOR_LIB_PATH=$(TOPDIR)/output/build/alexaClientSDK/source/alexa-rpi/lib/libsnsr.a \
						   -DSENSORY_KEY_WORD_DETECTOR_INCLUDE_DIR=$(TOPDIR)/output/build/alexaClientSDK/source/alexa-rpi/include

						   #-DKITTAI_KEY_WORD_DETECTOR=ON \
						   -DKITTAI_KEY_WORD_DETECTOR_LIB_PATH=$(TOPDIR)/output/build/alexaClientSDK/source/snowboy/lib/rpi/libsnowboy-detect.a \
						   -DKITTAI_KEY_WORD_DETECTOR_INCLUDE_DIR=$(TOPDIR)/output/build/alexaClientSDK/source/snowboy/include \

ALEXACLIENTSDK_CONF_OPTS +=\
						   -DPORTAUDIO=ON \
						   -DPORTAUDIO_LIB_PATH=$(TOPDIR)/output/target/usr/lib/libportaudio.so \
						   -DPORTAUDIO_INCLUDE_DIR=$(TOPDIR)/output/host/usr/arm-rockchip-linux-gnueabihf/sysroot/usr/include/

$(eval $(cmake-package))
endif


