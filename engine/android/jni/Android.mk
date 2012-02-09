LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#
LOCAL_CFLAGS := -DANDROID -DNATIVE_ACTIVITY $(LOCAL_CFLAGS)

#Set the name of the libX.so
LOCAL_MODULE    := shmup

# Include immediate subdirectories (for libraries)
include $(call all-subdir-makefiles)

#Building list of files specific to Android
ANDROID_SHMUP := $(wildcard $(LOCAL_PATH)/android/*.c) 
ANDROID_SHMUP := $(ANDROID_SHMUP:$(LOCAL_PATH)/%=%) 

#Building list of Shmup engine
SHMUP_ENGINE := $(wildcard $(LOCAL_PATH)/../../src/*.c) 
SHMUP_ENGINE := $(SHMUP_ENGINE:$(LOCAL_PATH)/%=%) 

#Combining the two list into one
LOCAL_SRC_FILES = $(ANDROID_SHMUP) $(SHMUP_ENGINE) 

LOCAL_LDLIBS    := -Wall -Wextra -Wmissing-prototypes -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -g
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
