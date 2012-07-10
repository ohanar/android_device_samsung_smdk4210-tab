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

TARGET_BOOTANIMATION_PRELOAD := true

TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_SMP := true
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_ARCH_VARIANT_CPU := cortex-a9
TARGET_ARCH_VARIANT_FPU := neon
ARCH_ARM_HAVE_TLS_REGISTER := true

TARGET_BOARD_PLATFORM := exynos4
TARGET_FAMILY := exynos4tab
TARGET_SOC := exynos4210
TARGET_BOOTLOADER_BOARD_NAME := smdk4210
TARGET_BOARD_INFO_FILE := device/samsung/exynos4tab-common/board-info.txt

COMMON_GLOBAL_CFLAGS += -DEXYNOS4210_ENHANCEMENTS
COMMON_GLOBAL_CFLAGS += -DSURFACEFLINGER_FORCE_SCREEN_RELEASE
COMMON_GLOBAL_CFLAGS += -DEXYNOS4210_TABLET

TARGET_NO_BOOTLOADER := true
TARGET_NO_RADIOIMAGE := true

TARGET_PROVIDES_INIT := true
TARGET_PROVIDES_INIT_TARGET_RC := true

BOARD_NAND_PAGE_SIZE := 4096
BOARD_NAND_SPARE_SIZE := 128
BOARD_KERNEL_BASE := 0x40000000

# Filesystem
TARGET_USERIMAGES_USE_EXT4 := true
BOARD_BOOTIMAGE_PARTITION_SIZE := 8388608
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 872415232
BOARD_USERDATAIMAGE_PARTITION_SIZE := 14138998784
BOARD_FLASH_BLOCK_SIZE := 1024

# Releasetools
# TODO: fix standard tools
TARGET_RELEASETOOL_OTA_FROM_TARGET_SCRIPT := ./device/samsung/exynos4tab-common/releasetools/c210_ota_from_target_files
TARGET_RELEASETOOL_IMG_FROM_TARGET_SCRIPT := ./device/samsung/exynos4tab-common/releasetools/c210_img_from_target_files

# Graphics
BOARD_EGL_CFG := device/samsung/exynos4tab-common/configs/egl.cfg
USE_OPENGL_RENDERER := true

# HWComposer
BOARD_USES_HWCOMPOSER := true
BOARD_USE_SECTVOUT := true
BOARD_USES_FIMGAPI := true

# OMX
BOARD_HAVE_CODEC_SUPPORT := SAMSUNG_CODEC_SUPPORT
COMMON_GLOBAL_CFLAGS += -DSAMSUNG_CODEC_SUPPORT
BOARD_NONBLOCK_MODE_PROCESS := true
BOARD_USE_STOREMETADATA := true
BOARD_USE_METADATABUFFERTYPE := true
BOARD_USES_MFC_FPS := true

# Audio
BOARD_USE_YAMAHAPLAYER := true
BOARD_USE_SAMSUNG_SEPARATEDSTREAM = true

# Bluetooth
BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_CSR := true
TARGET_CUSTOM_BLUEDROID := ../../../device/samsung/exynos4tab-common/bluetooth.c

# Vold
BOARD_VOLD_MAX_PARTITIONS := 11
BOARD_VOLD_EMMC_SHARES_DEV_MAJOR := true
BOARD_VOLD_DISC_HAS_MULTIPLE_MAJORS := true

# Recovery
TARGET_RECOVERY_INITRC := device/samsung/exynos4tab-common/recovery/init.rc
BOARD_CUSTOM_RECOVERY_KEYMAPPING := ../../device/samsung/exynos4tab-common/recovery/recovery_keys.c
TARGET_RECOVERY_PIXEL_FORMAT := "BGRA_8888"
BOARD_HAS_SDCARD_INTERNAL := true
BOARD_HAS_LARGE_FILESYSTEM := true
BOARD_HAS_NO_MISC_PARTITION := true
BOARD_HAS_NO_SELECT_BUTTON := true
BOARD_USES_MMCUTILS := true

# Wifi
BOARD_WLAN_DEVICE                := ath6kl
WPA_SUPPLICANT_VERSION           := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_ath6kl
WIFI_DRIVER_MODULE_PATH          := "/system/lib/modules/ath6kl.ko"
WIFI_DRIVER_MODULE_NAME          := ath6kl
WIFI_DRIVER_LOADER_DELAY         := 1000000

# Charging Mode (LPM)
BOARD_CHARGING_MODE_BOOTING_LPM := "/sys/class/power_supply/battery/batt_lp_charging"

# Many shipped exynos4tab devices have defective eMMC chips (VYL00M fwrev 0x19)
# Prevent usage of ERASE commands in recovery on these boards.
# This is redundant for our recovery since the kernel has MMC_CAP_ERASE
# disabled for mshci.c, however it makes nightly ZIPs safer to flash
# from kernels that still have MMC_CAP_ERASE enabled.
BOARD_SUPPRESS_EMMC_WIPE := true

# Kernel
TARGET_KERNEL_SOURCE := kernel/samsung/smdk4210-tab

CLEAN_MODULES:
	arm-eabi-strip --strip-debug `find $(KERNEL_MODULES_OUT) -name *.ko`

TARGET_KERNEL_MODULES := CLEAN_MODULES

BOARD_CUSTOM_BOOTIMG_MK := device/samsung/exynos4tab-common/bootimg.mk

# Use the non-open-source parts, if they're present
-include vendor/samsung/exynos4tab-common/BoardConfigVendor.mk
