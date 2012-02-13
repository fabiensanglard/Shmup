LOCAL_PATH := $(call my-dir)

#We need to save LOCAL_PATH since it is likely to be modified by the sub-modules.
SAVED_PATH := $(LOCAL_PATH)

#Building openal and libpng static libraries
include $(call all-subdir-makefiles)

#Clear everything and restore LOCAL_PATH
include $(CLEAR_VARS)
LOCAL_PATH := $(SAVED_PATH)


# Define ANDROID among other things. This is mandatory so Shmup engine can compile properly.
LOCAL_CFLAGS += -DANDROID -DNATIVE_ACTIVITY 

#Set the name of the libX.so (libshmup.so)
LOCAL_MODULE    := shmup





#Building list of files specific to Android
ANDROID_SHMUP := $(wildcard $(LOCAL_PATH)/android/*.c) 
ANDROID_SHMUP := $(ANDROID_SHMUP:$(LOCAL_PATH)/%=%) 




#Building list of Shmup engines source files.
SHMUP_ENGINE := $(wildcard $(LOCAL_PATH)/../../src/*.c) 
SHMUP_ENGINE := $(SHMUP_ENGINE:$(LOCAL_PATH)/%=%) 
#Android stores asset in a compressed pak file, hence we have a special filesystem and we should NOT use the default filesystem.c
toRemove:= filesystem.c
empty:=
SHMUP_ENGINE:= $(subst $(toRemove),$(empty),$(SHMUP_ENGINE))
#We don't need unzip.c on this platform either
#toRemove:= unzip.c
#SHMUP_ENGINE:= $(subst $(toRemove),$(empty),$(SHMUP_ENGINE))

# Thumb-2 is floating-point capable and intruction are smaller: No more ARM 
#LOCAL_ARM_MODE   := arm

#Combining the two list into one
LOCAL_SRC_FILES = $(ANDROID_SHMUP) $(SHMUP_ENGINE) 

#Adding the path for the openAL headers
LOCAL_C_INCLUDES := $(LOCAL_PATH)/openal/Include 

#Compiler flags !!!
LOCAL_LDLIBS    := -Wall -Wextra -Wmissing-prototypes -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -z -lOpenSLES 
LOCAL_STATIC_LIBRARIES := libzip libpng android_native_app_glue 
LOCAL_SHARED_LIBRARIES := libopenal

#Collect all variables since last CLEAR_VARS and compile the shared .so library
include $(BUILD_SHARED_LIBRARY)

#Build native glue static library as well.
$(call import-module,android/native_app_glue)
