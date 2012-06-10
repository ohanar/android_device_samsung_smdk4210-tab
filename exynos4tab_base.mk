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

DEVICE_PACKAGE_OVERLAYS += $(LOCAL_PATH)/overlay

# Init files
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/init.smdk4210.rc:root/init.smdk4210.rc \
    $(LOCAL_PATH)/init.smdk4210.usb.rc:root/init.smdk4210.usb.rc \
    $(LOCAL_PATH)/lpm.rc:root/lpm.rc \
    $(LOCAL_PATH)/ueventd.smdk4210.rc:root/ueventd.smdk4210.rc

# Audio
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/asound.conf:system/etc/asound.conf

# Vold and Storage
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/vold.fstab:system/etc/vold.fstab

# Bluetooth configuration file
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/PSConfig_8811.psr:system/etc/PSConfig_8811.psr

# Wifi
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf

PRODUCT_PROPERTY_OVERRIDES += \
    wifi.interface=wlan0 \
    wifi.supplicant_scan_interval=45

# Gps
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/gps.conf:system/etc/gps.conf \
    $(LOCAL_PATH)/configs/gps.xml:system/etc/gps.xml \
    $(LOCAL_PATH)/configs/sirfgps.conf:system/etc/sirfgps.conf

# Packages
PRODUCT_PACKAGES += \
    com.android.future.usb.accessory \
    librs_jni \
    SamsungServiceMode \
    Torch \
    bccmd

# HAL
PRODUCT_PACKAGES += \
    lights.exynos4 \
    libhwconverter \
    libs5pjpeg \
    libfimg

# MFC API
PRODUCT_PACKAGES += \
    libsecmfcapi

# OMX
PRODUCT_PACKAGES += \
    libstagefrighthw \
    libseccscapi \
    libsecbasecomponent \
    libsecosal \
    libSEC_OMX_Resourcemanager \
    libSEC_OMX_Core \
    libSEC_OMX_Vdec \
    libOMX.SEC.AVC.Decoder \
    libOMX.SEC.M4V.Decoder \
    libOMX.SEC.WMV.Decoder \
    libOMX.SEC.VP8.Decoder \
    libSEC_OMX_Venc \
    libOMX.SEC.AVC.Encoder \
    libOMX.SEC.M4V.Encoder \
    libSEC_OMX_Adec \
    libOMX.SEC.MP3.Decoder

# Filesystem management tools
PRODUCT_PACKAGES += \
    make_ext4fs \
    setup_fs \
    static_busybox

# These are the hardware-specific features
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/base/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/base/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/base/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/base/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/base/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/base/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/base/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/base/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    frameworks/base/data/etc/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml

# idc
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/usr/idc/S3C24XX_TouchScreen.idc:system/usr/idc/S3C24XX_TouchScreen.idc \
    $(LOCAL_PATH)/usr/idc/melfas_ts.idc:system/usr/idc/melfas_ts.idc \
    $(LOCAL_PATH)/usr/idc/mxt224_ts_input.idc:system/usr/idc/mxt224_ts_input.idc \
    $(LOCAL_PATH)/usr/idc/exynos4_ts.idc:system/usr/idc/exynos4_ts.idc \
    $(LOCAL_PATH)/usr/idc/sec_e-pen.idc:system/usr/idc/sec_e-pen.idc \
    $(LOCAL_PATH)/usr/idc/sec_touchscreen.idc:system/usr/idc/sec_touchscreen.idc

# keylayout
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/usr/keylayout/Vendor_04e8_Product_7021.kl:system/usr/keylayout/Vendor_04e8_Product_7021.kl \
    $(LOCAL_PATH)/usr/keylayout/samsung-keypad.kl:system/usr/keylayout/samsung-keypad.kl \
    $(LOCAL_PATH)/usr/keylayout/sec_e-pen.kl:system/usr/keylayout/sec_e-pen.kl \
    $(LOCAL_PATH)/usr/keylayout/sec_jack.kl:system/usr/keylayout/sec_jack.kl \
    $(LOCAL_PATH)/usr/keylayout/sec_key.kl:system/usr/keylayout/sec_key.kl \
    $(LOCAL_PATH)/usr/keylayout/sec_keyboard.kl:system/usr/keylayout/sec_keyboard.kl

# Wifi Firmware
# from kernel.org
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/firmware/ath6k/AR6003.1/hw2.1.1/endpointping.bin:system/etc/firmware/ath6k/AR6003.1/hw2.1.1/endpointping.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003.1/hw2.1.1/bdata.SD32.bin:system/etc/firmware/ath6k/AR6003.1/hw2.1.1/bdata.SD32.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003.1/hw2.1.1/bdata.SD31.bin:system/etc/firmware/ath6k/AR6003.1/hw2.1.1/bdata.SD31.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003.1/hw2.1.1/otp.bin:system/etc/firmware/ath6k/AR6003.1/hw2.1.1/otp.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003.1/hw2.1.1/data.patch.bin:system/etc/firmware/ath6k/AR6003.1/hw2.1.1/data.patch.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003.1/hw2.1.1/athwlan.bin:system/etc/firmware/ath6k/AR6003.1/hw2.1.1/athwlan.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003.1/hw2.1.1/bdata.WB31.bin:system/etc/firmware/ath6k/AR6003.1/hw2.1.1/bdata.WB31.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw2.0/bdata.SD32.bin:system/etc/firmware/ath6k/AR6003/hw2.0/bdata.SD32.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw2.0/otp.bin.z77:system/etc/firmware/ath6k/AR6003/hw2.0/otp.bin.z77 \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw2.0/bdata.SD31.bin:system/etc/firmware/ath6k/AR6003/hw2.0/bdata.SD31.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw2.0/athwlan.bin.z77:system/etc/firmware/ath6k/AR6003/hw2.0/athwlan.bin.z77 \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw2.0/data.patch.bin:system/etc/firmware/ath6k/AR6003/hw2.0/data.patch.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw2.0/bdata.WB31.bin:system/etc/firmware/ath6k/AR6003/hw2.0/bdata.WB31.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw2.1.1/endpointping.bin:system/etc/firmware/ath6k/AR6003/hw2.1.1/endpointping.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw2.1.1/bdata.SD32.bin:system/etc/firmware/ath6k/AR6003/hw2.1.1/bdata.SD32.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw2.1.1/bdata.SD31.bin:system/etc/firmware/ath6k/AR6003/hw2.1.1/bdata.SD31.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw2.1.1/fw-2.bin:system/etc/firmware/ath6k/AR6003/hw2.1.1/fw-2.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw2.1.1/otp.bin:system/etc/firmware/ath6k/AR6003/hw2.1.1/otp.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw2.1.1/data.patch.bin:system/etc/firmware/ath6k/AR6003/hw2.1.1/data.patch.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw2.1.1/athwlan.bin:system/etc/firmware/ath6k/AR6003/hw2.1.1/athwlan.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw2.1.1/bdata.WB31.bin:system/etc/firmware/ath6k/AR6003/hw2.1.1/bdata.WB31.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw1.0/bdata.SD32.bin:system/etc/firmware/ath6k/AR6003/hw1.0/bdata.SD32.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw1.0/otp.bin.z77:system/etc/firmware/ath6k/AR6003/hw1.0/otp.bin.z77 \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw1.0/bdata.SD31.bin:system/etc/firmware/ath6k/AR6003/hw1.0/bdata.SD31.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw1.0/athwlan.bin.z77:system/etc/firmware/ath6k/AR6003/hw1.0/athwlan.bin.z77 \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw1.0/data.patch.bin:system/etc/firmware/ath6k/AR6003/hw1.0/data.patch.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6003/hw1.0/bdata.WB31.bin:system/etc/firmware/ath6k/AR6003/hw1.0/bdata.WB31.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6002/eeprom.data:system/etc/firmware/ath6k/AR6002/eeprom.data \
    $(LOCAL_PATH)/firmware/ath6k/AR6002/athwlan.bin.z77:system/etc/firmware/ath6k/AR6002/athwlan.bin.z77 \
    $(LOCAL_PATH)/firmware/ath6k/AR6002/eeprom.bin:system/etc/firmware/ath6k/AR6002/eeprom.bin \
    $(LOCAL_PATH)/firmware/ath6k/AR6002/data.patch.hw2_0.bin:system/etc/firmware/ath6k/AR6002/data.patch.hw2_0.bin

PRODUCT_CHARACTERISTICS := tablet

PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=131072 \
    hwui.render_dirty_regions=false

PRODUCT_TAGS += dalvik.gc.type-precise

# Set default USB interface
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/media_profiles.xml:system/etc/media_profiles.xml

# Feature live wallpaper
PRODUCT_COPY_FILES += \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml

PRODUCT_PACKAGES += \
    LiveWallpapers \
    LiveWallpapersPicker \
    VisualizationWallpapers

$(call inherit-product, frameworks/base/build/tablet-dalvik-heap.mk)

# Include exynos4 platform specific parts
TARGET_HAL_PATH := hardware/samsung/exynos4/hal
TARGET_OMX_PATH := hardware/samsung/exynos/multimedia/openmax
$(call inherit-product, hardware/samsung/exynos4210.mk)
