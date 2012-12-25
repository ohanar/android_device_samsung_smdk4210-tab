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

$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)
$(call inherit-product, vendor/cm/config/common_full_tablet_wifionly.mk)
$(call inherit-product-if-exists, vendor/samsung/smdk4210-tab/vendor.mk)

# include a bunch of resources
PRODUCT_AAPT_CONFIG := normal large xlarge mdpi tvdpi hdpi
PRODUCT_LOCALES += mdpi tvdpi hdpi

# Init files
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/init.smdk4210.rc:root/init.smdk4210.rc \
    $(LOCAL_PATH)/init.smdk4210.usb.rc:root/init.smdk4210.usb.rc \
    $(LOCAL_PATH)/lpm.rc:root/lpm.rc \
    $(LOCAL_PATH)/fstab.smdk4210:root/fstab.smdk4210 \
    $(LOCAL_PATH)/ueventd.smdk4210.rc:root/ueventd.smdk4210.rc

# Vold and Storage
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/etc/vold.fstab:system/etc/vold.fstab

# Bluetooth configuration file
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/etc/PSConfig_8811.psr:system/etc/PSConfig_8811.psr

# Wifi
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/etc/wifi/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf

PRODUCT_PROPERTY_OVERRIDES += \
    wifi.interface=wlan0 \
    wifi.supplicant_scan_interval=45

# Gps
$(call inherit-product, device/common/gps/gps_us_supl.mk)
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/etc/gps.cer:system/etc/gps.cer

# Packages
PRODUCT_PACKAGES += \
    libsurfaceflinger_client \
    com.android.future.usb.accessory \
    librs_jni \
    Torch \
    macloader \
    bccmd \
    Smdk4210TabSettings

# HAL
PRODUCT_PACKAGES += \
    audio.a2dp.default \
    audio.primary.exynos4 \
    audio.usb.default \
    gralloc.exynos4 \
    hwcomposer.exynos4 \
    lights.exynos4 \
    libaudiohw_legacy \
    libhwconverter \
    libs5pjpeg \
    libfimg

# Charger
PRODUCT_PACKAGES += \
    charger \
    charger_res_images

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
    frameworks/native/data/etc/android.hardware.audio.low_latency.xml:system/etc/permissions/android.hardware.audio.low_latency.xml \
    frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml

# modified keylayout for keyboard dock
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/usr/keylayout/sec_keyboard.kl:system/usr/keylayout/sec_keyboard.kl

PRODUCT_PACKAGES += \
    sec_keyboard

PRODUCT_CHARACTERISTICS := tablet

PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=131072 \
    hwui.render_dirty_regions=false

PRODUCT_TAGS += dalvik.gc.type-precise

# Set default USB interface
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/etc/media_profiles.xml:system/etc/media_profiles.xml \
    $(LOCAL_PATH)/configs/etc/media_codecs.xml:system/etc/media_codecs.xml

# Feature live wallpaper
PRODUCT_COPY_FILES += \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml

PRODUCT_PACKAGES += \
    LiveWallpapers \
    LiveWallpapersPicker \
    VisualizationWallpapers

$(call inherit-product, frameworks/native/build/tablet-7in-hdpi-1024-dalvik-heap.mk)

# Include exynos4 platform specific parts
TARGET_HAL_PATH := hardware/samsung/exynos4/hal
TARGET_OMX_PATH := hardware/samsung/exynos/multimedia/openmax
$(call inherit-product-if-exists, hardware/samsung/exynos4210.mk)
