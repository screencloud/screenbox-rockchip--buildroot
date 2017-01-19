# RK1108 BSP packages
# Author : Cody Xie <cody.xie@rock-chips.com>

ifeq ($(BR2_PACKAGE_RK1108_FIRMWARE),y)
RK1108_FIRMWARE_FILES+=package/rockchip/rk1108-firmware/dsp/firmware/rkdsp.bin
RK1108_FIRMWARE_LIBS+=package/rockchip/rk1108-firmware/dsp/lib/libdpp.so
RK1108_FIRMWARE_INITSCRIPT+=package/rockchip/rk1108-firmware/dsp/S11dsp
RK1108_STAGING_LIBS+=package/rockchip/rk1108-firmware/dsp/lib/libdpp.so
RK1108_STAGING_INCLUDE_DIR+=package/rockchip/rk1108-firmware/dsp/include/dsp
endif

ifneq ($(RK1108_FIRMWARE_FILES)$(RK1108_FIRMWARE_LIBS)$(RK1108_FIRMWARE_INITSCRIPT),)
define RK1108_FIRMWARE_INSTALL_TARGET_FILES
$(foreach firmware,$(RK1108_FIRMWARE_FILES), \
	$(INSTALL) -D -m 0644 $(firmware) \
		$(TARGET_DIR)/lib/firmware/
)
$(foreach lib,$(RK1108_FIRMWARE_LIBS), \
	$(INSTALL) -D -m 0755 $(lib) \
		$(TARGET_DIR)/usr/lib/
)
$(foreach init,$(RK1108_FIRMWARE_INITSCRIPT), \
	$(INSTALL) -D -m 0755 $(init) \
		$(TARGET_DIR)/etc/init.d/
)
endef
endif

ifneq ($(RK1108_STAGING_INCLUDE_DIR)$(RK1108_STAGING_LIBS),)
RK1108_FIRMWARE_INSTALL_STAGING = YES
define RK1108_FIRMWARE_INSTALL_STAGING_FILES
$(foreach l,$(RK1108_STAGING_LIBS), \
	$(INSTALL) -D -m 0755 $(l) \
		$(STAGING_DIR)/usr/lib/
)
$(foreach d,$(RK1108_STAGING_INCLUDE_DIR), \
	rm -rf $(STAGING_DIR)/usr/include/$(shell basename $(d)); \
	cp -a $(d) $(STAGING_DIR)/usr/include/$(shell basename $(d))$(sep)
)
endef
endif

ifneq ($(RK1108_FIRMWARE_INSTALL_TARGET_FILES),)
define RK1108_FIRMWARE_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/lib/firmware
	$(RK1108_FIRMWARE_INSTALL_TARGET_FILES)
endef
endif

ifneq ($(RK1108_FIRMWARE_INSTALL_STAGING_FILES),)
define RK1108_FIRMWARE_INSTALL_STAGING_CMDS
	$(RK1108_FIRMWARE_INSTALL_STAGING_FILES)
endef
endif

$(eval $(generic-package))
