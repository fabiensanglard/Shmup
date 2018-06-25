/*
SHMUP is a 3D Shoot 'em up game inspired by Treasure Ikaruga

This file is part of SHMUP.

SHMUP is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SHMUP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SHMUP.  If not, see <https://www.gnu.org/licenses/>.
*/

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
