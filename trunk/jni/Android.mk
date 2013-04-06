LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := nativecb
LOCAL_SRC_FILES := nativecb.cpp

LOCAL_CFLAGS    := \
    -mandroid \
	-DTARGET_OS=android -D__ANDROID__ \
	-isystem $(SYSROOT)/usr/include

LOCAL_LDLIBS    := -llog

LOCAL_C_INCLUDES := $(LOCAL_PATH)/mstl $(LOCAL_PATH)/db $(LOCAL_PATH)/util

LOCAL_STATIC_LIBRARIES := mstl db util

include $(BUILD_SHARED_LIBRARY)

include jni/universalchardet/Android.mk

include jni/zlib/Android.mk

include jni/minizip/Android.mk

include jni/zzip/Android.mk

include jni/mstl/Android.mk

include jni/util/Android.mk

include jni/db/Android.mk
