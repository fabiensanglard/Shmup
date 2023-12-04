#include <jni.h>
#include "native_app_glue.h"
#include "../native_URL.h"
#include "../log.h"
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
