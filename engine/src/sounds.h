/*
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
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/    
/*
 *  sounds.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-06-06.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#ifndef FD_SOUNDS
#define FD_SOUNDS

#include "wavfile.h"
#include "globals.h"
#include "target.h"

//WIN32 debug
#define USE_OPENAL
#ifdef WIN32
#undef USE_OPENAL
#endif



#define SND_PLASMA		0
#define SND_EXPLOSION	1
#define SND_GHOST_LAUNCH 2
#define SND_ENEMY_SHOT 3


#ifdef WIN32
#include "al.h"
#elif defined (SHMUP_TARGET_ANDROID)
    #include "AL/al.h"
#else
#include "OpenAL/al.h"
#endif

typedef struct sound_t
{
	//uchar loaded;
	//char* path;
	
	soundInfo_t metaData;
	uchar* data;
	
	int size;


	ALenum format;
	ALuint alBuffer;


	int lastTimePlayed ;
	
} sound_t;


int SND_Init(void);
void SND_UpdateRecord(void);
void SND_FinalizeRecord(void);
void SND_PlaySound(int sndId);

#endif
