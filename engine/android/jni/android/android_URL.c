#include <jni.h>
#include <android_native_app_glue.h>
#include "../../../src/native_URL.h"

extern JNIEnv* env;
extern jobject*  activityObject;

void goToURL(char* url){
	jstring jstr = (*env)->NewStringUTF(env,url);
	jclass clazz = (*env)->GetObjectClass(env,activityObject);
	jmethodID goToWebsite = (*env)->GetMethodID(env,clazz, "goToWebsite", "(Ljava/lang/String;)V");
	(*env)->CallObjectMethod(env,jstr, goToWebsite);
}
