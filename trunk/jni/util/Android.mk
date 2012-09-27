LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := util
LOCAL_SRC_FILES := \
	u_base64_decoder.cpp \
	u_block_file.cpp \
	u_bit_stream.cpp \
	u_byte_stream.cpp \
	u_exception.cpp \
	u_http.cpp \
	u_misc.cpp \
	u_path.cpp \
	u_progress.cpp \
	u_zlib_ostream.cpp\
	u_zstream.cpp

LOCAL_CFLAGS    := \
	 -mandroid \
	-DTARGET_OS=android -D__ANDROID__ \
	-isystem $(SYSROOT)/usr/include
	-DNO_PREFETCH=1

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../mstl $(LOCAL_PATH)/../zzip $(LOCAL_PATH)/../minizip

LOCAL_STATIC_LIBRARIES := mstl zzip zlib minizip

include $(BUILD_SHARED_LIBRARY)
