#
# Makefile to build the jni library using real programming languages
#

LOCAL_PATH:= $(call my-dir)

############# BASS
include $(CLEAR_VARS)
LOCAL_MODULE := bass
LOCAL_SRC_FILES := ../../bass24-android/libs/$(TARGET_ARCH_ABI)/libbass.so
LOCAL_EXPORT_C_INCLUDES := ../../bass
include $(PREBUILT_SHARED_LIBRARY)

############# THEM STUFF
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../../bass24-android
LOCAL_SHARED_LIBRARIES := bass
LOCAL_MODULE    := libdemo
LOCAL_CFLAGS    := -Werror -ffast-math -O3 -funroll-loops
LOCAL_SRC_FILES := test3d.c sync/data.c sync/device.c sync/track.c
LOCAL_LDLIBS    += -llog -lGLESv2 -landroid -ldl -lz
LOCAL_ARM_MODE := arm
include $(BUILD_SHARED_LIBRARY)





