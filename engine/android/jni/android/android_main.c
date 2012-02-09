/*
 *
 * Notes:
 *
 * zlib is a must since all asset are compressed in an Android apk.
 * ndk-build and ndk-stack are invaluable tools.
 * Eclipse and build system detect automatically when the library has been recompiled.
 * printf are delayed so don't rely on them to debug. Use ndk-stack and -sym.
*/
#include <stdio.h>
#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include "../../../src/dEngine.h"

#define  LOG_TAG    "net.fabiensanglard.native"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define printf(fmt,args...)  __android_log_print(ANDROID_LOG_INFO  ,LOG_TAG, fmt, ##args)


/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {

	printf("Hello, Native World-11!\n");
	sleep(1);

	//Init everything except for the rendering system.
	dEngine_Init();



	while(1)
		sleep(1);
}
