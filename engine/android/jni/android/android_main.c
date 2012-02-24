/*
 *
 * Notes:
 *
 * zlib is a must since all asset are compressed in an Android apk.
 * ndk-build and ndk-stack are invaluable tools.
 * Eclipse and build system detect automatically when the library has been recompiled.
 * printf are delayed so don't rely on them to debug. Use ndk-stack and -sym.
 * EDIT: printf are not delayed but IGNORED !!! Be very careful to have the #define printf everywhere so you don't
 *       call the stdio printf: If you fall for this you won't see any trace.
 * I cannot get the assetManage and AAssetManager_openDir to work, all I can see is three files in "/": androidManifest.xml, classes.dex and ressources.arsc
 * It seems AAssetManager_openDir only list files. So "" returned nothings and so did "assets", "assets/data" did return
 * my two files.
 *
 * Getting closer to completion now. It seems the native AAssetManager_open refuses to load PNGs since they are already compressed.
 * http://ponystyle.com/blog/2010/03/26/dealing-with-asset-compression-in-android-apps/, aapt (Android Asset Packaging Tool) is messing them up.
 *
 * I was completely wrong about AAssetManager_open it can load anything, we just have to omit "assets" at the beginning and start with "data" right away.
 *
 *
 *  A very annoying thing with Eclipse NDK is that we compile with ndk-build which is outside Eclipse. In order to have Eclipse detect
 *  that the .so lib has been recompiled I had to check Preferences -> General -> Workspace - > "refresh using native hooks or polling"
 *  Even with that it is still not perfect: Eclipse misses the lib recompilation.
 *
 *  Just fixed a bug on Android. It was the consequence of a char wraparound. The wrap around was not an issue on signed char (windowd,ios and macosx).
 *  ...but it was a real problem on Android (crashing the game).
 *
 *
 *  Labels:
 *  X Done
 *    Pending
 *  / Will not fix
 *
 *
 *  TODO: X Fix the main loop and make it runs at 60Hz
 *  TODO: / Add OpenAL support.
 *  TODO: X Add music playback support
 *
 *  Super crap, the OpenES 1.1 implementation does not support an SL_DATALOCATOR_ADDRESS data locator with an SL_DATALOCATOR_OUTPUTMIX mixer:
 *  Source data locator 0x2 not supported with SL_DATALOCATOR_OUTPUTMIX sink !!
 *
 *  I have to redo the sound system with buffer queue...
 *
 *  Enqueue from SLBufferQueueItf is using the 1.0.1 spec. So the Android implementation is neither 1.0.1 nor 1.1, somewhere inbetween and not really there
 *  either since it doesn't implement any of the profile.......What was the point to adopt a standard ?!?!?
 *
 *  OpenSL spec code sample are broken and buggy (stack allocated context ??!!, the context address is not even passed to the callback)
 *
 *  TODO: X Build sound system with OpenSL EL
 *  TODO: X Change package to have net.fabiensanglard.shmup.
 *  TODO: X Fix inputs, the ship is not moving fast enough on Android
 *  TODO:   On Android Lite, add a link to the full version.
 *  TODO:   Fix bug when demo is requested and it starts a new game.
 *  TODO:   Fix score bug.
 *  TODO:   Increase scores.
 *  TODO: X Add back to menu functionality.
 *
 *
 *  Testing on Samsung Galaxy Tab. Everything is fine except for the control, moving is almost impossible. Time to dive into ALooper,Input Method Framework (IMF) and native_app_glue.
 *  It seems the design of Looper is very close to Windows Message Loop (http://msdn.microsoft.com/en-us/library/ms644928%28VS.85%29.aspx):
 *
 *  http://webcache.googleusercontent.com/search?q=cache%3aJUl_wh4lTkgJ%3arxwen.blogspot.com/2010/08/looper-and-handler-in-android.html%20complete%20tutorial%20looper%20in%20android&cd=8&hl=en&ct=clnk
 *
 *  Fixed issues: Rely on historicalEvent in order to trace move gesture is really really a BAD IDEA. I am tracing movement myself now.
 *  TODO: X Fix user input (I though I was missing events but actually I was using the wrong API method to trace mouvments.
 *  TODO:   Lower sound effects or Increase music sound volume
 *  TODO: X Build Shmup Lite
 *
 *
 *
 * JNI ISSUE :
 * ===========
 *  New error now when trying to call Java method from my native code: NDK JNI ERROR: non-VM thread making JNI calls
 *  http://developer.android.com/guide/practices/design/jni.html is a good ressource.
 *
 *  It seems the JNIEnv is not really valid when we receive it in the game thread. We need to attach the thread to the VM (AttachCurrentThread) and create a new JNIEnv
 *  http://android.wooyd.org/JNIExample/files/JNIExample.pdf
 *
 *  SNAAAAP !! Even after attaching and getting a valid JNIEnv it seems we still don't have the right classLoader !!!!!!
 *
 *http://blog.tewdew.com/post/6852907694/using-jni-from-a-native-activity
 *
 *
 *  TODO: X The music system is unable to start at a determined time within the music. Need to a SEEK interface in the OpenES implementation otherwise music in level2 is the same as level 1.
 *  TODO:   Fix thibault bug.
 *
 *
 *  TODO: X Create icons for Android Shmup and Android Shmup Lite
 *
 *
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
#include "../../../src/io_interface.h"
#include "../../../src/menu.h"
#include "../../../src/timer.h"
#include "../../../src/native_URL.h"

#include "android_display.h"
#include "android_filesystem.h"

// ANDROID_LOG_TAG must be defined via a compiler flag in Android.mk. This is done so
// Shmup and ShmupLite can use the same codebase.
#ifdef SHMUP_VERSION_LIMITED
	#define ANDROID_LOG_TAG "net.fabiensanglard.shmupLite"
#else
	#define ANDROID_LOG_TAG "net.fabiensanglard.shmup"
#endif

#define  LOG_TAG    		ANDROID_LOG_TAG
#define  LOGI(...)  		__android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  		__android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  		__android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define printf(fmt,args...) __android_log_print(ANDROID_LOG_INFO  ,LOG_TAG, fmt, ##args)

int gameOn = 0;
void AND_SHMUP_Finish(){
	engine_term_display();
	//shutdownAudio();
	Log_Printf("Warning the audio system is not being shutdown.");
	gameOn = 0;
	exit(0);
}


#include "android/asset_manager.h"

void ListDirectory(AAssetManager*      assetManager, const char* dirName){
	int fileCount=0;
	//Make sure the directory where we will be writing (logs, replays) exists.
			const char* assetURL;
			AAssetDir* directory = AAssetManager_openDir(assetManager,dirName);

			AAssetDir_rewind(directory);

			while((assetURL = AAssetDir_getNextFileName(directory)) != 0)
			{
				fileCount++;
				LOGE(assetURL);
			}
			//fflush(stdout);

			AAssetDir_close(directory);

			LOGI("Directory \"%s\" has %d entries.\n",dirName,fileCount);
}


io_event_s shmupEvent;
int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {

	size_t i;

	size_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;

	//System input (sound control, back home, ...).
	int32_t keFlags = AKeyEvent_getFlags(event);
	if (keFlags & AKEY_EVENT_FLAG_FROM_SYSTEM){
			Log_Printf("AKEY_EVENT_FLAG_FROM_SYSTEM\n");


			int32_t keyCode = AKeyEvent_getKeyCode(event);
			/* The only system input we are interested in is the BACK button:
			 * - If we are in the action phase it will bring us back to the main
			 * menu.
			 * - If we are in the menu, we exit the application
			 */
			if (keyCode == AKEYCODE_BACK && action == AMOTION_EVENT_ACTION_UP)
			{
				Log_Printf("engine_handle_input keyCode=%d\n",keyCode);
				if (engine.sceneId == 0){
					AND_SHMUP_Finish();
					return 1;
				}
				else if (engine.requiredSceneId != 0 && engine.sceneId != 0){
					MENU_Set(MENU_HOME);
					engine.requiredSceneId=0;
					return 1;
				}

			}
			return 0;
	}



	if (action != AMOTION_EVENT_ACTION_UP   &&
		action != AMOTION_EVENT_ACTION_DOWN &&
		action != AMOTION_EVENT_ACTION_MOVE
			){
	//	Log_Printf("[engine_handle_input] This action is not recognized (%d).",action);
		return 0;
	}


	if (action == AMOTION_EVENT_ACTION_UP)
	{
		 //printf("[event] AMOTION_EVENT_ACTION_UP");

		 shmupEvent.type = IO_EVENT_ENDED;
		 shmupEvent.position[X] = AMotionEvent_getX( event, 0 );
		 shmupEvent.position[Y] = AMotionEvent_getY( event, 0 );
	//	 Log_Printf("[engine_handle_input] UP   Pos [%.0f %.0f] \n",shmupEvent.position[X],shmupEvent.position[Y]);

		 IO_PushEvent(&shmupEvent);
		 return 1;
	}
	else if (action == AMOTION_EVENT_ACTION_DOWN){
		//printf("[event] AMOTION_EVENT_ACTION_DOWN");

		shmupEvent.type = IO_EVENT_BEGAN;
		shmupEvent.position[X] = AMotionEvent_getX( event, 0 );
		shmupEvent.position[Y] = AMotionEvent_getY( event, 0 );
		shmupEvent.previousPosition[X] = shmupEvent.position[X];
		shmupEvent.previousPosition[Y] = shmupEvent.position[Y];
	//	Log_Printf("[engine_handle_input] DOWN Pos [%.0f %.0f] \n",shmupEvent.position[X],shmupEvent.position[Y]);
		IO_PushEvent(&shmupEvent);
		return 1;
	}


	// We don't care about multi-touch so no need to look at other fingers: Removing this loop
	// ...we will always look at the pointerId=0 since it is the first finger to have touched the screen.

	//size_t pointerCount =  AMotionEvent_getPointerCount(event);
	//for (i = 0; i < pointerCount; i++){

		//size_t pointerId = AMotionEvent_getPointerId(event, i);


		shmupEvent.type = IO_EVENT_MOVED;
		shmupEvent.previousPosition[X] = shmupEvent.position[X] ;
		shmupEvent.previousPosition[Y] = shmupEvent.position[Y] ;
		shmupEvent.position[X] = AMotionEvent_getX( event, 0 );
		shmupEvent.position[Y] = AMotionEvent_getY( event, 0 );

		IO_PushEvent(&shmupEvent);
	//}

        return 1;
}



static void engine_handle_cmd(struct android_app* state, int32_t cmd) {
    //struct engine* engine = (struct engine*)app->userData;

    ANativeActivity*    activity  ;
    AAssetManager*      assetManager;
    activity = state->activity;
    assetManager = activity->assetManager;

    switch (cmd) {
        case APP_CMD_SAVE_STATE:
        	LOGI("APP_CMD_SAVE_STATE");
            break;
        case APP_CMD_INIT_WINDOW:
        	LOGI("APP_CMD_INIT_WINDOW");
        	window = state->window;
        	engine_init_display();
        	engine_draw_frame();
            break;
        case APP_CMD_TERM_WINDOW:
        	LOGI("APP_CMD_TERM_WINDOW");
            // The window is being hidden or closed, clean it up.

            break;
        case APP_CMD_GAINED_FOCUS:
        	LOGI("APP_CMD_GAINED_FOCUS");
            break;
        case APP_CMD_LOST_FOCUS:
        	printf("APP_CMD_LOST_FOCUS");
        	AND_SHMUP_Finish();
            break;
        case APP_CMD_WINDOW_RESIZED:
        	LOGI("APP_CMD_WINDOW_RESIZED");
        	//window = state->window;
        	//engine_term_display();
        	//engine_init_display();
        	break;
        case APP_CMD_CONFIG_CHANGED:
        	printf("APP_CMD_CONFIG_CHANGED");
        	window = state->window;
        	engine_term_display();
        	engine_init_display();
        	break;

    }
}

JNIEnv* env = NULL;
#ifdef SHMUP_VERSION_LIMITED
	char* className = "net.fabiensanglard.shmuplite.ShmupLiteActivity" ;
#else
	char* className = "net.fabiensanglard.shmup.Launcher" ;
#endif
//char* className = "java/lang/Object" ;
//char* className = "	android/app/NativeActivity";
char* methodName = "goToWebsite";
jobject  activityClass ;
jmethodID goToWebsite ;
void registerEnvironmentAndActivity(ANativeActivity* activity){

	JavaVM* vm = activity->vm;
	JNIEnv *jni;

	(**vm).AttachCurrentThread ( activity->vm , &jni , NULL ) ;

	env = jni ;

	jclass activityClass = (*jni)->FindClass(jni,"android/app/NativeActivity");
	if (!activityClass){
		LOGE("Unable to find class 'android/app/NativeActivity'.\n");
		return;
	}

	LOGE("Found android/app/NativeActivity class.\n");

	jmethodID getClassLoader = (*jni)->GetMethodID(jni,activityClass,"getClassLoader", "()Ljava/lang/ClassLoader;");
	if (!getClassLoader){
			LOGE("Unable to find method getClassLoader from 'android/app/NativeActivity'.\n");
			return;
	}

	LOGE("Found getClassLoader method in android/app/NativeActivity class.\n");

	jobject cls = (*jni)->CallObjectMethod(jni,activity->clazz, getClassLoader);

	if (!cls){
		LOGE("Faild to retrieve the ClassLoader object by calling android.app.NativeActivity.getClassLoader().\n");
		return;
	}

	LOGE("We now have a valid classloader object instance (cls) but we cannot call a method on it yet.\n");


	jclass classLoader = (*jni)->FindClass(jni,"java/lang/ClassLoader");
	if (!classLoader){
		LOGE("Unable to find class 'java/lang/ClassLoader'.\n");
		return;
	}

	LOGE("Found java/lang/ClassLoader class.\n");

	jmethodID findClass = (*jni)->GetMethodID(jni,classLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

	if (!findClass){
		LOGE("Unable to find method loadClass from 'java/lang/ClassLoader'.\n");
		return;
	}

	LOGE("Found loadClass method from java/lang/ClassLoader class.\n");

	jstring strClassName = (*jni)->NewStringUTF(jni,className);


	activityClass = (jclass)(*jni)->CallObjectMethod(jni,cls, findClass, strClassName);
	if (!activityClass){
		LOGE("Unable to find class '%s' using ClassLoader.loadClass(\"%s\").\n",className,className);
		return;
	}

	LOGE("Found '%s' class.\n",className);


	goToWebsite = (*jni)->GetStaticMethodID(jni, activityClass, methodName, "(Ljava/lang/String;)V");
	if (!goToWebsite){
		LOGE("Unable to find method %d in class %s.\n",methodName,className);
		return;
	}
	else
		LOGE("Found method %s: ready to call it.\n",methodName);

}



/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {

	app_dummy();

	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;

	LOGE("PRE  registerEnvironmentAndActivity");
	registerEnvironmentAndActivity(state->activity);
	LOGE("POST registerEnvironmentAndActivity");

	SND_Android_Init(state->activity->assetManager);


	LOGE("[FS_AndroidPreInitFileSystem]\n");



	FS_AndroidPreInitFileSystem(state);

	//Init everything except for the rendering system.

	renderer.materialQuality = MATERIAL_QUALITY_HIGH;
	renderer.statsEnabled = 0;

#ifdef SHMUP_VERSION_LIMITED
	engine.licenseType = LICENSE_LIMITED;
#else
	engine.licenseType = LICENSE_FULL;
#endif


	dEngine_Init();

	gameOn = 1;


	int ident;
	int events;
	struct android_poll_source* source;
	//Loop for ever, pump event and dispatch them.

	while (gameOn)
	{
	    // Read all pending events.


		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident = ALooper_pollAll(0, NULL, &events,(void**)&source)) >= 0)
		{
			// Process this event.
			if (source != NULL)
					source->process(state, source);

			// Check if we are exiting.
			if (state->destroyRequested != 0) {

				break;
				gameOn = 0;
			}
		}

		int frameStart = E_Sys_Milliseconds();
		engine_draw_frame();
		int frameEnd = E_Sys_Milliseconds();
	//	LOGE("[android_main] FRAME\n");

		unsigned int timeToSleep = timediff - (frameEnd-frameStart);

		//LOGE("[android_main] tts=%u\n",timeToSleep);

		// In the event engine_draw_frame took more time than timediff we end up with a
		// very high number. Set it to the maximum time instead.
		if (timeToSleep > 17)
			timeToSleep = 17;

		timeToSleep *= 1000;
		usleep( timeToSleep );

	 }
}


