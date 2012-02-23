//
//  sound_openAL.h
//  dEngine
//
//  Created by fabien sanglard on 12-02-14.
//  Copyright (c) 2012 Memset Software. All rights reserved.
//

#ifndef dEngine_sound_openAL_h
#define dEngine_sound_openAL_h

#include "sounds.h"



void SND_BACKEND_Upload(sound_t* sound, int soundID);
void SND_BACKEND_Init(void );
void SND_BACKEND_Play(int sndId);
#endif
