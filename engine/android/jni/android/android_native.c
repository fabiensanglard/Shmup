

// music.h placeholders
void SND_InitSoundTrack(char* filename,unsigned int startAt){}
void SND_StartSoundTrack(void){}
void SND_StopSoundTrack(void){}
void SND_PauseSoundTrack(void){}
void SND_ResumeSoundTrack(void){}


// native_service.h
int  Native_RetrieveListOf(char replayList[10][256]){ return 0;}
void Native_UploadFileTo(char path[256]){}
void Action_ShowGameCenter(void* tag){}
void Native_UploadScore(unsigned int score){}
void Native_LoginGameCenter(void){}


//ITextureloader.h
#include "../../src/texture.h"
void loadNativePNG(texture_t* tmpTex){}
