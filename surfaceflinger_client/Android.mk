LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    surfaceflinger_client.cpp

LOCAL_SHARED_LIBRARIES := 

LOCAL_MODULE:= libsurfaceflinger_client

include $(BUILD_SHARED_LIBRARY)
