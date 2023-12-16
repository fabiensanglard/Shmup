# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.22.1)

# Declares and names the project.

project("shmup")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-security -DANDROID -DNATIVE_ACTIVITY")


set(BASE "../../../../../src")
file(GLOB SOURCES ${BASE}/*.h ${BASE}/*.c)
file(GLOB ANDROID_BACKENDS ${BASE}/android/*.h ${BASE}/android/*.c)

add_library( # Sets the name of the library.
        shmup

        # Sets the library as a shared library.
        SHARED

        ${BASE}/libpng/png.c
        ${BASE}/libpng/pngerror.c
        ${BASE}/libpng/pngget.c
        ${BASE}/libpng/pngmem.c
        ${BASE}/libpng/pngpread.c
        ${BASE}/libpng/pngread.c
        ${BASE}/libpng/pngrio.c
        ${BASE}/libpng/pngrtran.c
        ${BASE}/libpng/pngrutil.c
        ${BASE}/libpng/pngset.c
        ${BASE}/libpng/pngtrans.c
        ${BASE}/libpng/pngwio.c
        ${BASE}/libpng/pngwrite.c
        ${BASE}/libpng/pngwtran.c
        ${BASE}/libpng/pngwutil.c
        ${BASE}/libpng/arm/arm_init.c
        ${BASE}/libpng/arm/filter_neon.S
        ${BASE}/libpng/arm/filter_neon_intrinsics.c
        ${BASE}/libpng/arm/palette_neon_intrinsics.c
        ${SOURCES}
        ${ANDROID_BACKENDS}
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