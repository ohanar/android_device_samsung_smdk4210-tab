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
	$(LOCAL_PATH)/lpm.rc:root/lpm.rc \
	$(LOCAL_PATH)/init.smdkc210.common.rc:root/init.smdkc210.common.rc \
	$(LOCAL_PATH)/ueventd.smdkc210.rc:root/ueventd.smdkc210.rc

# Audio
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/configs/asound.conf:system/etc/asound.conf

# Vold and Storage
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/vold.fstab:system/etc/vold.fstab

# Bluetooth configuration files
PRODUCT_COPY_FILES += \
	system/bluetooth/data/main.le.conf:system/etc/bluetooth/main.conf

# Wifi
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/configs/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf

PRODUCT_PROPERTY_OVERRIDES += \
	wifi.interface=wlan0 \
	wifi.supplicant_scan_interval=45

# Gps
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/configs/gps.conf:system/etc/gps.conf \
	$(LOCAL_PATH)/configs/sirfgps.conf:system/etc/sirfgps.conf

# Packages
PRODUCT_PACKAGES += \
    com.android.future.usb.accessory \
    librs_jni \
    SamsungServiceMode

# Audio packages
PRODUCT_PACKAGES += \
    audio.a2dp.default \
    libaudioutils

# Sensors
PRODUCT_PACKAGES += \
	lights.s5pc210

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
    $(LOCAL_PATH)/usr/idc/s5pc210_ts.idc:system/usr/idc/s5pc210_ts.idc \
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

PRODUCT_CHARACTERISTICS := tablet

#PRODUCT_PROPERTY_OVERRIDES += \
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
