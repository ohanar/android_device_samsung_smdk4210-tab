#
# Copyright (C) 2012 The Android Open-Source Project
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

BOARD_USES_GENERIC_AUDIO := false

TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_SMP := true
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_ARCH_VARIANT_CPU := cortex-a9
ARCH_ARM_HAVE_NEON := true
ARCH_ARM_HAVE_TLS_REGISTER := true
EXYNOS4210_ENHANCEMENTS := true

ifdef EXYNOS4210_ENHANCEMENTS
COMMON_GLOBAL_CFLAGS += -DEXYNOS4210_ENHANCEMENTS
COMMON_GLOBAL_CFLAGS += -DSURFACEFLINGER_FORCE_SCREEN_RELEASE
endif

TARGET_BOARD_PLATFORM := s5pc210
TARGET_BOOTLOADER_BOARD_NAME := smdkc210
TARGET_BOARD_INFO_FILE := device/samsung/c210-common/board-info.txt

TARGET_NO_BOOTLOADER := true
TARGET_NO_RADIOIMAGE := true

TARGET_PROVIDES_INIT := true
TARGET_PROVIDES_INIT_TARGET_RC := true
TARGET_RECOVERY_INITRC := device/samsung/c210-common/recovery/init.rc

BOARD_NAND_PAGE_SIZE := 4096 -s 128
BOARD_KERNEL_PAGESIZE := 4096
BOARD_KERNEL_BASE := 0x40000000
BOARD_KERNEL_CMDLINE := console=ttySAC2,115200 consoleblank=0

# Filesystem
TARGET_USERIMAGES_USE_EXT4 := true
BOARD_BOOTIMAGE_PARTITION_SIZE := 8388608
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 872415232
BOARD_USERDATAIMAGE_PARTITION_SIZE := 14138998784
BOARD_FLASH_BLOCK_SIZE := 1024

# Releasetools
TARGET_RELEASETOOL_OTA_FROM_TARGET_SCRIPT := ./device/samsung/c210-common/releasetools/c210_ota_from_target_files
TARGET_RELEASETOOL_IMG_FROM_TARGET_SCRIPT := ./device/samsung/c210-common/releasetools/c210_img_from_target_files

# Graphics
BOARD_EGL_CFG := device/samsung/c210-common/configs/egl.cfg
USE_OPENGL_RENDERER := true
COMMON_GLOBAL_CFLAGS += -DUSES_LEGACY_EGL -DMISSING_EGL_PIXEL_FORMAT_YV12 -DMISSING_GRALLOC_BUFFERS -DMISSING_EGL_EXTERNAL_IMAGE

# HWComposer
BOARD_USES_HWCOMPOSER := true

# OMX
BOARD_HAVE_CODEC_SUPPORT := SAMSUNG_CODEC_SUPPORT
BOARD_USES_PROPRIETARY_OMX := SAMSUNG
COMMON_GLOBAL_CFLAGS += -DSAMSUNG_CODEC_SUPPORT

# Audio
BOARD_USE_YAMAHAPLAYER := true
BOARD_USES_AUDIO_LEGACY := true

# Bluetooth
BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_BCM := true

# Vold
BOARD_VOLD_MAX_PARTITIONS := 11
BOARD_VOLD_EMMC_SHARES_DEV_MAJOR := true
BOARD_VOLD_DISC_HAS_MULTIPLE_MAJORS := true

# MTP
BOARD_MTP_DEVICE := "/dev/usb_mtp_gadget"

# Recovery
BOARD_CUSTOM_GRAPHICS := ../../../device/samsung/c210-common/recovery/graphics.c
BOARD_USES_MMCUTILS := true
BOARD_HAS_NO_MISC_PARTITION := true
BOARD_HAS_NO_SELECT_BUTTON := true

# Wifi
#BOARD_WLAN_DEVICE                := ar6000
#WPA_SUPPLICANT_VERSION           := VER_0_6_X
#BOARD_WPA_SUPPLICANT_DRIVER      := AR6000
#WIFI_DRIVER_MODULE_PATH          := "/system/lib/modules/ar6000.ko"
#WIFI_DRIVER_MODULE_NAME          := ar6000
#BOARD_WEXT_NO_COMBO_SCAN         := true

# Use the non-open-source parts, if they're present
-include vendor/samsung/c210/BoardConfigVendor.mk

BOARD_CUSTOM_BOOTIMG_MK := device/samsung/c210-common/shbootimg.mk
