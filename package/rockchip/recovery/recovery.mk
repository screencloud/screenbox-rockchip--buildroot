################################################################################
#
# Rockchip Recovery For Linux
#
################################################################################

RECOVERY_SITE = $(TOPDIR)/../recovery
RECOVERY_VERSION = develop
RECOVERY_SITE_METHOD = local

RECOVERY_LICENSE_FILES = NOTICE
RECOVERY_LICENSE = Apache V2.0
RECOVERY_KERNEL_DIR = $(TOPDIR)/../kernel
RECOVERY_KERNEL_IMAGE = $(RECOVERY_KERNEL_DIR)/arch/arm64/boot/Image
RECOVERY_MK_KERNEL_IMAGE=$(TOPDIR)/../rkbin/tools/mkkrnlimg
RECOVERY_MKBOOTIMG=$(RECOVERY_KERNEL_DIR)/scripts/mkbootimg
RECOVERY_RESOURCEIMG=$(RECOVERY_KERNEL_DIR)/resource.img
RECOVERY_CMDLINNE=buildvariant=userdebug
RECOVERY_OS_PATCH_LEVEL=2018.04.27
RECOVERY_OS_VERSION=1.0

RECOVERY_DEPENDENCIES = libdrm libpng
RECOVERY_BUILD_OPTS= \
	-I$(STAGING_DIR)/usr/include/libdrm \
	--sysroot=$(STAGING_DIR) \
	-fPIC

RECOVERY_MAKE = \
	$(MAKE) BOARD=$(BR2_ARCH) \

RECOVERY_MAKE_OPTS = \
	CC="$(TARGET_CC) $(RECOVERY_BUILD_OPTS)" \

define RECOVERY_CONFIGURE_CMDS
	# Do nothing
endef

define RECOVERY_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(RECOVERY_MAKE) -C $(@D) \
		$(RECOVERY_MAKE_OPTS)
endef

define RECOVERY_INSTALL_TARGET_CMDS
        $(INSTALL) -D -m 755 $(@D)/recovery $(TARGET_DIR)/usr/bin/
endef

ifeq ($(BR2_PACKAGE_RECOVERY),y)
define RECOVERY_IMAGE_PACK
	mkdir -p $(TARGET_DIR)/res/images
	cp $(BUILD_DIR)/recovery-$(RECOVERY_VERSION)/res/images/* $(TARGET_DIR)/res/images/
	$(HOST_DIR)/usr/bin/mkbootfs $(TARGET_DIR) | $(HOST_DIR)/usr/bin/minigzip > $(BINARIES_DIR)/ramdisk-recovery.img
	$(RECOVERY_MKBOOTIMG) --kernel $(RECOVERY_MK_KERNEL_IMAGE) --ramdisk $(BINARIES_DIR)/ramdisk-recovery.img --second $(RECOVERY_RESOURCEIMG) --os_version $(RECOVERY_OS_VERSION) --os_patch_level $(RECOVERY_OS_PATCH_LEVEL) --cmdline $(RECOVERY_CMDLINNE) --output $(BINARIES_DIR)/recovery.img
	$(RECOVERY_MK_KERNEL_IMAGE) $(BINARIES_DIR)/ramdisk-recovery.img $(BINARIES_DIR)/recovery.img
endef

TARGET_FINALIZE_HOOKS += RECOVERY_IMAGE_PACK
endif

$(eval $(autotools-package))
