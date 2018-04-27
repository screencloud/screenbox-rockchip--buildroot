################################################################################
#
# mkbootfs
#
################################################################################

HOST_MKBOOTFS_VERSION = master
HOST_MKBOOTFS_SITE = $(TOPDIR)/../external/mkbootfs
HOST_MKBOOTFS_SITE_METHOD = local
HOST_MKBOOTFS_LICENSE = Apache V2.0
HOST_MKBOOTFS_LICENSE_FILES = NOTICE

define HOST_MKBOOTFS_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define HOST_MKBOOTFS_INSTALL_CMDS                                               
        $(INSTALL) -D -m 755 $(@D)/mkbootfs $(HOST_DIR)/usr/bin/mkbootfs
endef

$(eval $(host-generic-package)) 
                                                                                
