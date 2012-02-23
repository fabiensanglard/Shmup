LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)


LOCAL_MODULE    := libpng

#Building the list of libPNG source.
LIB_PNG_SOURCES    := $(wildcard $(LOCAL_PATH)/*.c)
LIB_PNG_SOURCES    := $(LIB_PNG_SOURCES:$(LOCAL_PATH)/%=%) 

LOCAL_SRC_FILES := $(LIB_PNG_SOURCES) 
	
# libpng uses libz
LOCAL_LDLIBS := -lz

# Thumb-2 is floating-point capable and intruction are smaller: No more ARM 
#LOCAL_ARM_MODE   := arm

include $(BUILD_STATIC_LIBRARY)
