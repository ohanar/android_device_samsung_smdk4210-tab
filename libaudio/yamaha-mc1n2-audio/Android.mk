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

ifeq ($(strip $(BOARD_USE_YAMAHA_MC1N2_AUDIO)),true)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	yamaha-mc1n2-audio.c \
	device/smdk4210.c

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := \
	libc \
	libcutils \
	libutils

ifeq ($(BOARD_HAS_EARPIECE), true)
  LOCAL_CFLAGS += -DHAS_EARPIECE
endif

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libyamaha-mc1n2-audio

include $(BUILD_SHARED_LIBRARY)

endif
