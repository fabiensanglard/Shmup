//
//  macosx_native.c
//  macosx
//
//  Created by fabien sanglard on 12-01-31.
//  Copyright (c) 2012 Memset Software. All rights reserved.
//

#include "globals.h"

void SND_InitSoundTrack(char* filename,unsigned int startAt){}
void SND_StartSoundTrack(void){}
void SND_StopSoundTrack(void){}
void SND_PauseSoundTrack(void){}
void SND_ResumeSoundTrack(void){}



int  Native_RetrieveListOf(char replayList[10][256]){return 0;}
void Native_UploadFileTo(char path[256]){}
void Action_ShowGameCenter(void* tag){}
void Native_UploadScore(uint score){}
void Native_LoginGameCenter(void){}

#include "texture.h"
void loadNativePNG(texture_t* tmpTex){}
