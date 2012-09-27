LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := minizip
LOCAL_SRC_FILES := \
	unzip.c \
	zip.c \
	ioapi.c

LOCAL_CFLAGS    := \
	 -mandroid \
	-DTARGET_OS=android -D__ANDROID__ \
	-isystem $(SYSROOT)/usr/include \
	-DNO_PREFETCH=1

LOCAL_STATIC_LIBRARIES := zlib

include $(BUILD_SHARED_LIBRARY)
