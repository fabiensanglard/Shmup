#include <jni.h>
#include <android_native_app_glue.h>
#include "../../../src/native_URL.h"
#include "../../../src/log.h"
//#include "stdlib.h"

extern JNIEnv* env;
extern jobject  activityClass;
extern jmethodID goToWebsite;
void goToURL(char* url){

	Log_Printf("url=%s\n",url);

	if (!goToWebsite){
		Log_Printf("No Java VM method defined. Cannot invocate !\n");
		return;
	}
	jstring jstr = (*env)->NewStringUTF(env,url);



	(*env)->CallStaticObjectMethod(env,activityClass,goToWebsite,jstr);
	//exit(0);




	//jclass clazz = (*env)->GetObjectClass(env,activityClass);
	//jmethodID goToWebsite = (*env)->GetMethodID(env,clazz, "goToWebsite", "(Ljava/lang/String;)V");

	//jmethodID goToWebsite = (*env)->GetStaticMethodID(env, activityClass, "goToWebsite", "(Ljava/lang/String;)V");

}
