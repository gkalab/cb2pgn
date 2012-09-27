LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := universalchardet
LOCAL_SRC_FILES := \
	CharDistribution.cpp \
	LangHebrewModel.cpp \
	LangThaiModel.cpp \
	nsHebrewProber.cpp \
	LangCyrillicModel.cpp \
	nsSBCSGroupProber.cpp \
	nsUniversalDetector.cpp \
	LangGreekModel.cpp \
	nsGB2312Prober.cpp \
	nsLatin1Prober.cpp \
	nsEUCJPProber.cpp \
	nsUTF8Prober.cpp \
	nsEscSM.cpp \
	nsEscCharsetProber.cpp \
	nsSJISProber.cpp \
	nsMBCSSM.cpp \
	LangHungarianModel.cpp \
	nsEUCTWProber.cpp \
	LangBulgarianModel.cpp \
	nsSBCharSetProber.cpp \
	JpCntx.cpp \
	nsCharSetProber.cpp \
	nsEUCKRProber.cpp \
	nsMBCSGroupProber.cpp \
	nsBig5Prober.cpp

LOCAL_CFLAGS    := \
	 -mandroid \
	-DTARGET_OS=android -D__ANDROID__ \
	-isystem $(SYSROOT)/usr/include
	-DNO_PREFETCH=1

LOCAL_STATIC_LIBRARIES :=

include $(BUILD_SHARED_LIBRARY)
