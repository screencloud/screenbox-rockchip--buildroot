# Rockchip's MPP(Multimedia Processing Platform) demo
MPP_LINUX_CPP_VERSION = master
MPP_LINUX_CPP_SITE = https://github.com/sliver-chen/mpp_linux_cpp.git
MPP_LINUX_CPP_SITE_METHOD = git

define MPP_LINUX_CPP_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/mpp_linux_demo -t $(TARGET_DIR)/usr/local/bin/
endef

$(eval $(cmake-package))
