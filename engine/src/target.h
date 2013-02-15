//
//  target.h
//  macosx
//
//  Created by fabien sanglard on 12-01-31.
//  Copyright (c) 2012 Memset Software. All rights reserved.
//

#ifndef macosx_target_h
#define macosx_target_h


/*
 
         The sole goal of this header is to define either of the following:
 
        SHMUP_TARGET_MACOSX
        SHMUP_TARGET_IOS
        SHMUP_TARGET_WINDOWS
        SHMUP_TARGET_ANDROID
 
 
 */


// Try to see if the target build is a Windows machine
#if defined( _WIN32 ) 
#define SHMUP_TARGET_WINDOWS
#endif


//Try to see if the target build is a macosx machine
/*
 http://sealiesoftware.com/blog/archive/2010/8/16/TargetConditionalsh.html
                              Mac OS X	 iOS device	        iOS simulator
 TARGET_OS_MAC	                1	         1	                 1
 TARGET_OS_IPHONE	            0	         1	                 1
 TARGET_OS_EMBEDDED	            0	         1	                 0
 TARGET_IPHONE_SIMULATOR	    0	         0	                 1
 */

#if defined (__APPLE__) 
#include <TargetConditionals.h>


   #if TARGET_OS_MAC == 1 &&  TARGET_OS_IPHONE == 0
         #define SHMUP_TARGET_MACOSX
   #endif

   #if TARGET_OS_IPHONE==1 
      #define SHMUP_TARGET_IOS

   #endif

//__APPLE__
#endif

#ifdef ANDROID
    #define SHMUP_TARGET_ANDROID
#endif

#ifdef LINUX
    #define SHMUP_TARGET_LINUX
#endif

//macosx_target_h
#endif

