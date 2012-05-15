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

LOCAL_PATH := $(call my-dir)

$(INSTALLED_BOOTIMAGE_TARGET): $(INSTALLED_KERNEL_TARGET) $(INTERAL_RAMDISK_FILES) $(utilities) $(TARGET_RECOVERY_ROOT_TIMESTAMP)
	$(call pretty,"Boot image: $@")
	# setup lpm in ramdisk
	cp -f $(PRODUCT_OUT)/utilities/busybox $(TARGET_ROOT_OUT)/sbin/busybox
	mv $(TARGET_ROOT_OUT)/init $(TARGET_ROOT_OUT)/init2
	cp $(LOCAL_PATH)/lpm/init $(TARGET_ROOT_OUT)/init

	$(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(TARGET_ARCH) $(ARM_CROSS_COMPILE) CONFIG_INITRAMFS_SOURCE=$(ANDROID_BUILD_TOP)/$(TARGET_ROOT_OUT) $(TARGET_PREBUILT_INT_KERNEL_TYPE)
	$(ACP) $(KERNEL_BIN) $@
	
	# undo lpm in ramdisk: can interfere with the rest of the build system
	rm -f $(TARGET_ROOT_OUT)/sbin/busybox
	mv $(TARGET_ROOT_OUT)/init2 $(TARGET_ROOT_OUT)/init

# we depend on the previous make target to insure that the two build serially
$(INSTALLED_RECOVERYIMAGE_TARGET): $(INSTALLED_BOOTIMG_TARGET)
	$(call pretty,"Recovery image: $@")
	$(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(TARGET_ARCH) $(ARM_CROSS_COMPILE) CONFIG_INITRAMFS_SOURCE=$(ANDROID_BUILD_TOP)/$(TARGET_RECOVERY_ROOT_OUT) $(TARGET_PREBUILT_INT_KERNEL_TYPE)
	$(ACP) $(KERNEL_BIN) $@
