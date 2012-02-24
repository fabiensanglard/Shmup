#include <jni.h>
#include <android_native_app_glue.h>
#include "../../../src/native_URL.h"
#include "../../../src/log.h"
//#include "stdlib.h"

extern JNIEnv* env;
extern jobject  activityInstance;
extern jmethodID goToWebsite;
void goToURL(char* url){

	Log_Printf("url=%s\n",url);

	if (!goToWebsite){
		Log_Printf("No Java VM method defined. Cannot invocate !\n");
		return;
	}
	jstring jstr = (*env)->NewStringUTF(env,url);



	(*env)->CallVoidMethod(env,activityInstance,goToWebsite,jstr);


}
