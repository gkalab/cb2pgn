LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := zzip
LOCAL_SRC_FILES := \
	zzip-io.c \
	zzip-file.c \
	zzip-dir.c \
	zzip-zip.c \
	zzip-info.c \
	zzip-stat.c \
	zzip-err.c

LOCAL_CFLAGS    := \
	 -mandroid \
	-DTARGET_OS=android -D__ANDROID__ \
	-isystem $(SYSROOT)/usr/include
	-DNO_PREFETCH=1

LOCAL_STATIC_LIBRARIES := zlib

include $(BUILD_SHARED_LIBRARY)
