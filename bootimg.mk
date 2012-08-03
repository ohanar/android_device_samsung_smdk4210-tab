#
# Copyright (C) 2012 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

INSTALLED_BOOTIMAGE_TARGET ?= $(PRODUCT_OUT)/boot.img
INSTALLED_RECOVERYIMAGE_TARGET ?= $(PRODUCT_OUT)/recovery.img
INSTALLED_UNCOMPRESSED_RAMDISK_TARGET ?= $(PRODUCT_OUT)/ramdisk.cpio
recovery_uncompressed_ramdisk ?= $(PRODUCT_OUT)/ramdisk-recovery.cpio

$(INSTALLED_UNCOMPRESSED_RAMDISK_TARGET): $(MINIGZIP) \
    $(INTERNAL_RAMDISK_FILES)
	@echo "----- Making uncompressed ramdisk ------"
	$(MKBOOTFS) $(TARGET_ROOT_OUT) > $@

$(INSTALLED_BOOTIMAGE_TARGET): $(INSTALLED_KERNEL_TARGET) $(INSTALLED_UNCOMPRESSED_RAMDISK_TARGET)
	$(call pretty,"Target boot image: $@")
	$(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(TARGET_ARCH) $(ARM_CROSS_COMPILE) CONFIG_INITRAMFS_SOURCE=$(ANDROID_BUILD_TOP)/$(INSTALLED_UNCOMPRESSED_RAMDISK_TARGET) $(TARGET_PREBUILT_INT_KERNEL_TYPE)
	$(ACP) $(KERNEL_BIN) $@
	@echo -e ${CL_INS}"Made boot image: $@"${CL_RST}

# we depend on the previous make target to insure that the two build serially
$(INSTALLED_RECOVERYIMAGE_TARGET): $(INSTALLED_BOOTIMAGE_TARGET) $(recovery_uncompressed_ramdisk)
	@echo "----- Making recovery image ------"
	$(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(TARGET_ARCH) $(ARM_CROSS_COMPILE) CONFIG_INITRAMFS_SOURCE=$(ANDROID_BUILD_TOP)/$(recovery_uncompressed_ramdisk) $(TARGET_PREBUILT_INT_KERNEL_TYPE)
	$(ACP) $(KERNEL_BIN) $@
	@echo -e ${CL_INS}"Made recovery image: $@"${CL_RST}
	$(hide) $(call assert-max-image-size,$@,$(BOARD_RECOVERYIMAGE_PARTITION_SIZE),raw)
