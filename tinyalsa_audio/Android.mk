# Copyright (C) 2012 Paul Kocialkowski <contact@paulk.fr>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

LOCAL_PATH := $(call my-dir)

ifeq ($(strip $(BOARD_USE_TINYALSA_AUDIO)),true)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	audio_hw.c \
	audio_out.c \
	audio_in.c \
	audio_ril_interface.c \
	mixer.c

LOCAL_C_INCLUDES += \
	external/tinyalsa/include \
	external/expat/lib \
	system/media/audio_utils/include \
	system/media/audio_effects/include \
	hardware/tinyalsa-audio/include

LOCAL_SHARED_LIBRARIES := \
	libc \
	libcutils \
	libutils \
	libexpat \
	libtinyalsa \
	libaudioutils \
	libdl

ifeq ($(strip $(BOARD_USE_YAMAHA_MC1N2_AUDIO)),true)
  LOCAL_CFLAGS += -DYAMAHA_MC1N2_AUDIO
  LOCAL_C_INCLUDES += $(LOCAL_PATH)/../yamaha-mc1n2-audio/include
  LOCAL_SHARED_LIBRARIES += libyamaha-mc1n2-audio

  LOCAL_CFLAGS += -DYAMAHA_MC1N2_AUDIO_DEVICE=\"smdk4210\"

endif

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := audio.primary.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

include $(BUILD_SHARED_LIBRARY)

endif
