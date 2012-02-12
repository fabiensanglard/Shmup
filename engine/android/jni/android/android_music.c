#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <android/native_activity.h>
#include "../../src/log.h"

// music.h placeholders
/*
void SND_InitSoundTrack(char* filename,unsigned int startAt){}
void SND_StartSoundTrack(void){}
void SND_StopSoundTrack(void){}
void SND_PauseSoundTrack(void){}
void SND_ResumeSoundTrack(void){}
*/



SLEngineItf engineEngine;
SLObjectItf fdPlayerObject = NULL;
SLObjectItf outputMixObject = NULL;
SLPlayItf fdPlayerPlay;
SLSeekItf fdPlayerSeek;

static AAssetManager* assetManager;
void SND_Android_Init(AAssetManager* mgr)
{
	assetManager = mgr;
}

void SND_InitSoundTrack( char* str,unsigned int startAt)
{
    SLresult result;

    char * filename = str;

    Log_Printf("[SND_InitSoundTrack] %s.\n",str);


    // use asset manager to open asset by filename
    assert(NULL != assetManager);

    AAsset* asset = AAssetManager_open(assetManager, (const char *) filename, AASSET_MODE_UNKNOWN);


    // the asset might not be found
    if (NULL == asset) {
    	Log_Printf("[SND_InitSoundTrack] Could not find sound asset '%s'", filename);
        return;
    }

    // open asset as file descriptor
    off_t start, length;
    int fd = AAsset_openFileDescriptor(asset, &start, &length);
    assert(0 <= fd);
    AAsset_close(asset);

    // configure audio source
    SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
    SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
    SLDataSource audioSrc = {&loc_fd, &format_mime};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[2] = {SL_IID_SEEK, SL_IID_VOLUME};
    const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &fdPlayerObject, &audioSrc, &audioSnk,2, ids, req);
    assert(SL_RESULT_SUCCESS == result);

    // realize the player
    result = (*fdPlayerObject)->Realize(fdPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the play interface
    result = (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_PLAY, &fdPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);

    // get the seek interface
    result = (*fdPlayerObject)->GetInterface(fdPlayerObject, SL_IID_SEEK, &fdPlayerSeek);
    assert(SL_RESULT_SUCCESS == result);

    // enable whole file looping
    result = (*fdPlayerSeek)->SetLoop(fdPlayerSeek, SL_BOOLEAN_TRUE, 0, SL_TIME_UNKNOWN);
    assert(SL_RESULT_SUCCESS == result);
}

void SND_StartSoundTrack(void)
{
    SLresult result;

    // make sure the asset audio player was created
    if (NULL != fdPlayerPlay) {
        // set the player's state
        //result = (*fdPlayerPlay)->SetPlayState(fdPlayerPlay, isPlaying ? SL_PLAYSTATE_PLAYING : SL_PLAYSTATE_PAUSED);
        result = (*fdPlayerPlay)->SetPlayState(fdPlayerPlay, SL_PLAYSTATE_PLAYING);
        assert(SL_RESULT_SUCCESS == result);
    }
}

void SND_StopSoundTrack(void)
{
    SLresult result;

    // make sure the asset audio player was created
    if (NULL != fdPlayerPlay) {
        // set the player's state
        result = (*fdPlayerPlay)->SetPlayState(fdPlayerPlay, SL_PLAYSTATE_PAUSED);
        assert(SL_RESULT_SUCCESS == result);

    }
}
