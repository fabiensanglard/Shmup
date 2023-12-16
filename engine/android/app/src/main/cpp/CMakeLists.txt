# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.22.1)

# Declares and names the project.

project("shmup")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-security -DANDROID -DNATIVE_ACTIVITY")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -llog -landroid -lEGL")


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

file(GLOB SOURCES ../../../../../src/*.h ../../../../../src/*.c)
file(GLOB ANDROID_BACKENDS ../../../../../src/android/*.h ../../../../../src/android/*.c)
#file(GLOB LIBPNG ../../../../../src/libpng/*.h ../../../../../src/libpng/*.c)

add_library( # Sets the name of the library.
        shmup

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ../../../../../src/libpng/png.c
        ../../../../../src/libpng/pngerror.c
        ../../../../../src/libpng/pngget.c
        ../../../../../src/libpng/pngmem.c
        ../../../../../src/libpng/pngpread.c
        ../../../../../src/libpng/pngread.c
        ../../../../../src/libpng/pngrio.c
        ../../../../../src/libpng/pngrtran.c
        ../../../../../src/libpng/pngrutil.c
        ../../../../../src/libpng/pngset.c
        ../../../../../src/libpng/pngtest.c
        ../../../../../src/libpng/pngtrans.c
        ../../../../../src/libpng/pngwio.c
        ../../../../../src/libpng/pngwrite.c
        ../../../../../src/libpng/pngwtran.c
        ../../../../../src/libpng/pngwutil.c
        ../../../../../src/libpng/arm/arm_init.c
        ../../../../../src/libpng/arm/filter_neon.S
        ../../../../../src/libpng/arm/filter_neon_intrinsics.c
        ../../../../../src/libpng/arm/palette_neon_intrinsics.c

        ${SOURCES}
        ${ANDROID_BACKENDS}
#        ${LIBPNG}
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        shmup
        android
        EGL
        GLESv1_CM
        GLESv2
        z
        OpenSLES
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})