LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := zlib
LOCAL_SRC_FILES := \
	gzclose.c \
	gzlib.c \
	gzread.c \
	gzwrite.c \
	adler32.c \
	compress.c \
	crc32.c \
	deflate.c \
	infback.c \
	inffast.c \
	inflate.c \
	inftrees.c \
	trees.c \
	uncompr.c \
	zutil.c

LOCAL_CFLAGS    := \
	 -mandroid \
	-DTARGET_OS=android -D__ANDROID__ \
	-isystem $(SYSROOT)/usr/include \
	-DNO_PREFETCH=1

LOCAL_STATIC_LIBRARIES :=

include $(BUILD_SHARED_LIBRARY)
