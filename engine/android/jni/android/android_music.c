/*
 OpenSL available on 2.3.3

Android  "API Levels" :
=======================
android-3      -> Official Android 1.5   system images
android-4      -> Official Android 1.6   system images
android-5      -> Official Android 2.0   system images
android-6      -> Official Android 2.0.1 system images
android-7      -> Official Android 2.1   system images
android-8      -> Official Android 2.2   system images
android-9      -> Official Android 2.3   system images
android-10     -> Official Android 2.3.3 system images
android-11     -> Official Android 3.0   system images
android-12     -> Official Android 3.1   system images
android-13     -> Official Android 3.2   system images
android-14     -> Official Android 4.0   system images
android-15     -> Official Android 4.0.3 system images

NDK Compatibility :
===================

android-3:
    - C library: (<stdlib.h>, <stdio.h>, etc...)
    - <math.h>
    - Minimal C++: (<cstddef>,<new>,<utility>,<stl_pair.h>)
    - Android logs: liblog.so
    - zlib: (<zlib.h> and <zconf.h>)
    - ld dynamic linker: <dlfcn.h>

android-4:
    - OpenGL ES 1.0 & OpenGL ES 1.1 (on GPU equipped phones)
           - <GLES/gl.h>
           - <GLES/glext.h>

android-5:
    - OpenGL ES 2.0
         - <GLES2/gl2.h>
         - <GLES2/gl2ext.h>

android-8:
    - jnigraphics lib (Access framebuffer of the phone)
    -

android-9:
    - EGL lib (EGL config, eglSwapBuffers)
         - <EGL/egl.h>
         - <EGL/eglext.h>
    - OpenSL ES
         - <SLES/OpenSLES.h>
         - <SLES/OpenSLES_Platform.h>
         - <SLES/OpenSLES_Android.h>
         - <SLES/OpenSLES_AndroidConfiguration.h>

    - Android native application


android-14:
     - OpenMAX AL (native multimedia) WARNING, this has nothing to do with OpenAL !!!
         -  <OMXAL/OpenMAXAL.h>
         -  <OMXAL/OpenMAXAL_Platform.h>
     - OpenSL ES can now decoded to PCM


Based on those, Shmup should target Android API level 9. OpenAL and music playback should be implementabl on top of OpenSL.
It seems Android-10 is fixing a lot of issues in Android-9, also according to the stats from Google (http://developer.android.com/resources/dashboard/platform-versions.html)
There is very little to gain to go with 9:

Let's go with Android-10 (2.3.3 )

ADB USB Drivers is an absolute pain on Windows (works out of the box on MacOS X).
Eclipse C/C++ support is nice.
*/


#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>

#include <android/native_activity.h>

#include "../../src/log.h"
#include <assert.h>

static AAssetManager* assetManager;
void SND_Android_Init(AAssetManager* mgr)
{
	assetManager = mgr;
}

#include "../../src/music.h"
#include "../../src/sounds.h"
#include "../../src/sound_backend.h"

/*
 * Overall architecture: The sound system is expected to be able to play NUM_SOURCES sounds at the same time.
 * Additionally it must be able to play music.
 *
 * We create an audioPlayer for each channels plus one for music.
 *
 * Since there is no way to change the DataSource of a player, we have to create one each time a channel changes source.
 * Same thing for music, each time it is changed we have to create a new player.
 *
 */

//Reserve enough players for each channels
SLObjectItf openES_players[NUM_SOURCES];
unsigned char* wavSounds[NUM_SOURCES];

//Music player
SLObjectItf musicPlayerInterface = NULL;



SLEngineItf engineInterface;
SLObjectItf outputMixObject = NULL;
SLDataLocator_OutputMix loc_outmix;
SLDataSink audioSink;
/*
 * Create all the common part needed by the sound subsystem, only the players are missing.
 *
 */
void SND_BACKEND_Init(void ){

    SLresult result;
    SLObjectItf engineObject = NULL;


    memset(openES_players,0,sizeof(openES_players));

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    assert(SL_RESULT_SUCCESS == result);




    // create output mix, with environmental reverb specified as a non-required interface
    /*
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 1, ids, req);
    */
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);


    // configure audio sink, every player outputs are going there.
    loc_outmix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
    loc_outmix.outputMix   =  outputMixObject;


    audioSink.pLocator = &loc_outmix;

    //According to the specs, this is ignored if locatorType == SL_DATALOCATOR_OUTPUTMIX
    audioSink.pFormat = NULL;

}


// music.h placeholders

void SND_InitSoundTrack(char* filename,unsigned int startAt)
{
	 SLresult result;

	 Log_Printf("[SND_InitSoundTrack]\n");

	 if (filename[0] == '/')
		 filename++;

	//The music player goes in the lot NUM_SOURCES. Always.

	 AAsset* asset = AAssetManager_open(assetManager, (const char *) filename, AASSET_MODE_UNKNOWN);

	 //1. First let's create the DataSource
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

		// configure audio source with the file descriptor we just obtained.
		SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
		SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
		SLDataSource audioSrc = {&loc_fd, &format_mime};

	//2. Create the player and connect it to the DataSource and the Mixer sink

		 // create audio player with volume and seek interface available
		const SLInterfaceID ids[2] = {SL_IID_SEEK, SL_IID_VOLUME};
		const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
		result = (*engineInterface)->CreateAudioPlayer(engineInterface,&musicPlayerInterface , &audioSrc, &audioSink,2, ids, req);
		assert(SL_RESULT_SUCCESS == result);


		// realize the player
		result = (*musicPlayerInterface)->Realize(musicPlayerInterface, SL_BOOLEAN_FALSE);
		assert(SL_RESULT_SUCCESS == result);




}
void SND_StartSoundTrack(void){


	SLPlayItf playerInterface;


	SLresult result;

	Log_Printf("[SND_StartSoundTrack]\n");

	if(!musicPlayerInterface){
		Log_Printf("[SND_StopSoundTrack] No music player to start\n");
		return;
	}

	//Get the player interface so we can actually deal with it.
	result = (*musicPlayerInterface)->GetInterface(musicPlayerInterface, SL_IID_PLAY, &playerInterface);
	assert(SL_RESULT_SUCCESS == result);

	result = (*playerInterface)->SetPlayState(playerInterface,SL_PLAYSTATE_PLAYING);
	assert(SL_RESULT_SUCCESS == result);

}
void SND_StopSoundTrack(void){

	SLPlayItf playerInterface;


	SLresult result;

	Log_Printf("[SND_StopSoundTrack]\n");

	if(!musicPlayerInterface){
		Log_Printf("[SND_StopSoundTrack] No music player to stop\n");
		return;
	}

	//Get the player interface so we can actually deal with it.
	result = (*musicPlayerInterface)->GetInterface(musicPlayerInterface, SL_IID_PLAY, &playerInterface);
	assert(SL_RESULT_SUCCESS == result);


	//Stop will place the head at the beginning of the track
	result = (*playerInterface)->SetPlayState(playerInterface,SL_PLAYSTATE_STOPPED);
	assert(SL_RESULT_SUCCESS == result);

}

void SND_PauseSoundTrack(void){
	Log_Printf("SND_PauseSoundTrack is not implemented./n");
}
void SND_ResumeSoundTrack(void){
	Log_Printf("SND_ResumeSoundTrack is not implemented./n");
}

//BACKEND IMPLEMENTATION using buffer queue and callbacks.



/* Structure for passing information to callback function */

typedef struct CallbackContext_s {
   unsigned char*   pDataBase;    // Base adress of local audio data storage
   unsigned char*   pData;        // Current adress of local audio data storage
   SLuint32   size;
} CallbackContext_t;

CallbackContext_t openESContextes[NUM_SOURCES+1];

#define AUDIO_DATA_BUFFER_SIZE (1024*20)

//void BufferQueueCallback(SLBufferQueueItf queueItf,SLuint32 eventFlags,const void * pBuffer,SLuint32 bufferSize,SLuint32 dataUsed,void *pContext)
void BufferQueueCallback(SLAndroidSimpleBufferQueueItf queueItf, void *pContext)
{
	//We are never streaming, the buffer passed contains the full sound...
	return;
	/*
   SLresult res;
   CallbackContext_t *pCntxt = pContext;


   Log_Printf("[BufferQueueCallback] totalsize of sound=%u\n",pCntxt->size);
   Log_Printf("[BufferQueueCallback] base of sound=0x%X\n",pCntxt->pDataBase);
   Log_Printf("[BufferQueueCallback] ptr  of sound=0x%X\n",pCntxt->pData);
   Log_Printf("[BufferQueueCallback] remaining =%d\n",(pCntxt->pDataBase + pCntxt->size) - pCntxt->pData);

   int byteRead ;
   int bytesLeftInBuffer = (pCntxt->pDataBase + pCntxt->size) - pCntxt->pData;

   if (bytesLeftInBuffer > AUDIO_DATA_BUFFER_SIZE){
	   byteRead = AUDIO_DATA_BUFFER_SIZE;
   }
   else{
	   byteRead = bytesLeftInBuffer ;
	   if (byteRead <=0){ //Nothing valid to enqueue
		   Log_Printf("[BufferQueueCallback] Nothing to enqueue.\n");
		   pCntxt->pData = pCntxt->pDataBase;
		   return;
	   }
   }

   Log_Printf("[BufferQueueCallback] enqueuing %d bytes.\n",byteRead);

   res = (*queueItf)->Enqueue(queueItf, pCntxt->pData, byteRead);
   assert(SL_RESULT_SUCCESS == res);

   // Increase data pointer by appropriate size
   if (SL_RESULT_SUCCESS == res)
	   pCntxt->pData += byteRead;
*/

}

void SND_BACKEND_Upload(sound_t* sound, int soundID){

	Log_Printf("[SND_BACKEND_Upload] Start\n");

	// Since the application will free the data buffer containing the wav, we have to copy it here locally.
	wavSounds[soundID] = malloc(sound->size);
	memcpy(wavSounds[soundID],sound->data,sound->size);


	//Create player and its associated dataSource.
	SLDataLocator_AndroidSimpleBufferQueue pLocator = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

	Log_Printf("[SND_BACKEND_Upload] soundID=%d\n",soundID);

	SLDataFormat_PCM pFormat;
	pFormat.formatType = SL_DATAFORMAT_PCM;
	pFormat.numChannels = 1;
	pFormat.samplesPerSec = sound->metaData.sample_rate * 1000;
	pFormat.bitsPerSample = sound->metaData.sample_size*8;
	pFormat.containerSize = sound->metaData.sample_size*8; // This is probably used for the stride.
	pFormat.channelMask = 0;//SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
	pFormat.endianness = SL_BYTEORDER_LITTLEENDIAN; // But Wavs are stored in bigendian ?!?!?

	SLDataSource audioSource = {&pLocator, &pFormat};


	SLresult result;

	 // create audio player with volume and seek interface available
	const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
	const SLboolean req[1] = {SL_BOOLEAN_TRUE};
	result = (*engineInterface)->CreateAudioPlayer(engineInterface,&openES_players[soundID] , &audioSource, &audioSink,1, ids, req);
	//result = (*engineInterface)->CreateAudioPlayer(engineInterface,&openES_players[soundID] , &audioSource, &audioSink,0, NULL, NULL);
	assert(SL_RESULT_SUCCESS == result);

	SLObjectItf player =  openES_players[soundID];

	// realize the player
	result = (*player)->Realize(player, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);


	//Add the buffer queue stuff
	CallbackContext_t* context = &openESContextes[soundID];
	context->size = sound->size;
	context->pDataBase = context->pData = wavSounds[soundID];

	Log_Printf("[SND_BACKEND_Upload] totalsize of sound=%u\n",context->size);
	Log_Printf("[SND_BACKEND_Upload] base of sound=0x%X\n",context->pDataBase);
	Log_Printf("[SND_BACKEND_Upload] ptr  of sound=0x%X\n",context->pData);
	Log_Printf("[SND_BACKEND_Upload] remaining =%d\n",(context->pDataBase + context->size) - context->pData);



	SLPlayItf playerInterface;
	result = (*player)->GetInterface(player, SL_IID_PLAY, &playerInterface);
	assert(SL_RESULT_SUCCESS == result);


	SLBufferQueueItf bufferQueueItf;
	result = (*player)->GetInterface(player, SL_IID_BUFFERQUEUE, (void*)&bufferQueueItf);
	assert(SL_RESULT_SUCCESS == result);


//	result = (*bufferQueueItf)->RegisterCallback(bufferQueueItf, (void*)BufferQueueCallback, context);
//	assert(SL_RESULT_SUCCESS == result);


}

void SND_BACKEND_Play(int soundID){


	//Log_Printf("[SND_BACKEND_Play] %d.\n",soundID);

		CallbackContext_t* context = &openESContextes[soundID];


		SLObjectItf player =  openES_players[soundID];
		SLPlayItf playerInterface;


		if (!player)
		{
			Log_Printf("[SND_BACKEND_Play] No player for id=%d.\n",soundID-1);
			return;
		}

		SLresult result;



		//Get the player interface so we can actually deal with it.
		result = (*player)->GetInterface(player, SL_IID_PLAY, &playerInterface);
		assert(SL_RESULT_SUCCESS == result);

		//Stop the player and set it to play again.

		//Stop will place the head at the beginning of the track
		result = (*playerInterface)->SetPlayState(playerInterface,SL_PLAYSTATE_STOPPED);
		assert(SL_RESULT_SUCCESS == result);




		// Enqueue a buffer right away since it seems that the callback is only called when a buffer finish playing.
		// we need a first one to init the pumping

		SLBufferQueueItf bufferQueueItf;
		result = (*player)->GetInterface(player, SL_IID_BUFFERQUEUE, (void*)&bufferQueueItf);
		assert(SL_RESULT_SUCCESS == result);

		result = (*bufferQueueItf)->Clear(bufferQueueItf);
		assert(SL_RESULT_SUCCESS == result);

		//We need to reset the context pointer as well
	//	context->pData = context->pDataBase;

		result = (*playerInterface)->SetPlayState(playerInterface,SL_PLAYSTATE_PLAYING);
		assert(SL_RESULT_SUCCESS == result);

		int sizeToEnqueue = context->size;//AUDIO_DATA_BUFFER_SIZE > context->size ? context->size : AUDIO_DATA_BUFFER_SIZE ;
		//Log_Printf("[SND_BACKEND_Play] Will Queued %d bytes for sound %d.\n",sizeToEnqueue,soundID);
		result = (*bufferQueueItf)->Enqueue(bufferQueueItf, context->pDataBase, sizeToEnqueue);
		assert(SL_RESULT_SUCCESS == result);
		//Log_Printf("[SND_BACKEND_Play] Did Queued %d bytes for sound %d.\n",sizeToEnqueue,soundID);

	//	context->pData += sizeToEnqueue;



}

//SNAP SL_DATALOCATOR_ADDRESS are not support with a SL_DATALOCATOR_OUTPUTMIX associated to the sink.
/*
void SND_BACKEND_Upload(sound_t* sound, int soundID){

	Log_Printf("[SND_BACKEND_Upload] Start\n");

	// Since the application will free the data buffer containing the wav, we have to copy it here locally.
	wavSounds[soundID] = malloc(sound->size);
	memcpy(wavSounds[soundID],sound->data,sound->size);


	//Create player and its associated dataSource.
	SLDataLocator_Address pLocator;

	pLocator.locatorType = SL_DATALOCATOR_ADDRESS;
	pLocator.pAddress = wavSounds[soundID];
	pLocator.length = sound->size;

	Log_Printf("[SND_BACKEND_Upload] pLocator size=%d\n",sound->size);

	SLDataFormat_PCM pFormat;
	pFormat.formatType = SL_DATAFORMAT_PCM;
	pFormat.numChannels = 1;


	pFormat.samplesPerSec = sound->metaData.sample_rate * 1000;

	pFormat.bitsPerSample = sound->metaData.sample_size*8;
	pFormat.containerSize = sound->metaData.sample_size*8; // This is probably used for the stride.
	pFormat.channelMask = 0;//SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
	pFormat.endianness = SL_BYTEORDER_BIGENDIAN; // Wavs are stored in bigendian/

	SLDataSource audioSource = {&pLocator, &pFormat};




	SLresult result;

	 // create audio player with volume and seek interface available
	//const SLInterfaceID ids[2] = {SL_IID_SEEK, SL_IID_VOLUME};
	//const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
	//result = (*engineInterface)->CreateAudioPlayer(engineInterface,&openES_players[soundID] , &audioSource, &audioSink,2, ids, req);

	result = (*engineInterface)->CreateAudioPlayer(engineInterface,&openES_players[soundID] , &audioSource, &audioSink,0, NULL, NULL);

	assert(SL_RESULT_SUCCESS == result);

	SLObjectItf player =  openES_players[soundID];

	// realize the player
	result = (*player)->Realize(player, SL_BOOLEAN_FALSE);
	assert(SL_RESULT_SUCCESS == result);

}



void SND_BACKEND_Play(int soundID){



	SLObjectItf player =  openES_players[soundID];
	SLPlayItf playerInterface;


	SLresult result;



	//Get the player interface so we can actually deal with it.
	result = (*player)->GetInterface(player, SL_IID_PLAY, &playerInterface);
	assert(SL_RESULT_SUCCESS == result);

	//Stop the player and set it to play again.

	//Stop will place the head at the beginning of the track
	result = (*playerInterface)->SetPlayState(playerInterface,SL_PLAYSTATE_STOPPED);
	assert(SL_RESULT_SUCCESS == result);

	result = (*playerInterface)->SetPlayState(playerInterface,SL_PLAYSTATE_PLAYING);
	assert(SL_RESULT_SUCCESS == result);

}
*/
