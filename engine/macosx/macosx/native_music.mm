//
//  native_music.mm
//  macosx
//
//  Created by fabien sanglard on 12-02-04.
//  Copyright (c) 2012 Memset Software. All rights reserved.
//

#include <iostream>

#include "globals.h"


#ifdef __cplusplus
extern "C" {
#endif 
    
NSSound* sound;

void SND_InitSoundTrack(char* filename,unsigned int startAt)
{
    NSString* path = [NSString stringWithCString:filename encoding:NSUTF8StringEncoding];
    
    sound =[[NSSound alloc] initWithContentsOfFile:path byReference:NO];
   // sound = [NSSound alloc[initWithContentsOfFile:path byReference:NO]];
    
    
}

void SND_StartSoundTrack(void)
{
    [sound play];
}

void SND_StopSoundTrack(void)
{
    [sound stop];
    [sound release];
}

void SND_PauseSoundTrack(void)
{
    
}
void SND_ResumeSoundTrack(void)
{
    
}
    
#ifdef __cplusplus
}
#endif 
