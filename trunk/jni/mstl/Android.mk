LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := mstl
LOCAL_SRC_FILES := \
	fopencookie.c \
	open_memstream.c \
	fmemopen.c \
	m_algorithm.cpp \
	m_assert.cpp \
	m_backtrace.cpp \
	m_bit_functions.cpp \
	m_bitset.cpp \
	m_bitset_iterator.cpp \
	m_byte_buf.cpp \
	m_equiv_classes.cpp \
	m_exception.cpp \
	m_file.cpp \
	m_fstream.cpp \
	m_hash.cpp \
	m_ifstream.cpp \
	m_ios.cpp \
	m_istream.cpp \
	m_ofstream.cpp \
	m_ostream.cpp \
	m_sstream.cpp \
	m_string.cpp

LOCAL_CFLAGS    := -I$(LOCAL_PATH)/../stlport/stlport \
	 -mandroid \
	-DTARGET_OS=android -D__ANDROID__ \
	-isystem $(SYSROOT)/usr/include \
	-Wno-psabi \
	-DNO_PREFETCH=1

LOCAL_STATIC_LIBRARIES := stlport

include $(BUILD_SHARED_LIBRARY)
