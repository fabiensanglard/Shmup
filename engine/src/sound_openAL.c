//
//  sound_openAL.c
//  dEngine
//
//  Created by fabien sanglard on 12-02-14.
//  Copyright (c) 2012 Memset Software. All rights reserved.
//

#include "sound_backend.h"
#include "dEngine.h"
#include "log.h"

#ifdef WIN32
#include "al.h"
#include "alc.h"
#elif defined (SHMUP_TARGET_ANDROID)
#include "AL/al.h"
#include "AL/alc.h"
#else
#include "OpenAL/al.h"
#include "OpenAL/alc.h"
#endif


typedef struct sound_meta_s
{
    ALenum format;
    ALuint alBuffer;
    
} sound_meta_t;


ALuint sources[NUM_SOURCES];

ALCcontext* context;
ALCdevice* device;

sound_meta_t alMetadatas[NUM_SOURCES];


ALenum formatEquivalent[] = {AL_FORMAT_STEREO16 ,
                        AL_FORMAT_MONO16   ,
                        AL_FORMAT_STEREO8,
                        AL_FORMAT_MONO8};


void SND_BACKEND_Upload(sound_t* sound, int soundID)
{
    alGenBuffers(1, &(alMetadatas[soundID].alBuffer));
	alBufferData(alMetadatas[soundID].alBuffer, formatEquivalent[sound->format], sound->data, sound->size, sound->metaData.sample_rate );
    
}




void SND_BACKEND_OPENAL_GenerateChannels(void )
{
	int i;
	
	for (i=0; i < NUM_SOURCES; i++) 
	{
		alGenSources(1, &sources[i]);
	}
}


char *deviceList;
char *sound_devices[ 12 ];
ushort numSoundDevices, numDefaultSoundDevice;
void SND_BACKEND_OPENAL_GetDeviceList( void )
{
	char deviceName[ 256 ];
	
	//my_strlcpy( deviceName, s_device->string, sizeof( deviceName ) );
	if( alcIsExtensionPresent( NULL,"ALC_ENUMERATION_EXT") == AL_TRUE ) 
	{	
		// try out enumeration extension
		deviceList = (char *)alcGetString( NULL, ALC_DEVICE_SPECIFIER );
		
        Log_Printf("OpenAL SND_GetDeviceList.\n",deviceList);
        
		for( numSoundDevices = 0 ; numSoundDevices < 12 ; ++numSoundDevices ) 
		{
			sound_devices[ numSoundDevices ] = NULL;
		}
		
		for( numSoundDevices = 0 ; numSoundDevices < 12 ; ++numSoundDevices )
		{
			sound_devices[ numSoundDevices ] = deviceList;
			if( strcmp( sound_devices[ numSoundDevices ], deviceName ) == 0 )
			{
				numDefaultSoundDevice = numSoundDevices;
			}
			deviceList += strlen( deviceList );
			if( deviceList[ 0 ] == 0 )
			{
				if( deviceList[ 1 ] == 0 )
				{
					break;
				} 
				else 
				{
					deviceList += 1;
				}
			}
			
			
		} // End for numSoundDevices = 0 ; numSoundDevices < 12 ; ++numSoundDevices
	}
	
}




void SND_BACKEND_Init(void )
{
    SND_BACKEND_OPENAL_GetDeviceList();
	
	device = alcOpenDevice( NULL  );
	if( device == NULL )
	{
		Log_Printf( "Failed to Initialize OpenAL\n" );
		alcDestroyContext( context );
		context = 0;
		return;
	}
	
	// Create context(s)
	context = alcCreateContext( device, NULL );
	if( context == NULL )
	{
		Log_Printf( "Failed to initialize OpenAL\n" );
		alcDestroyContext( context );
		context = 0;
		return;
	}
	
	
	
	// Set active context
	alcGetError( device );
	alcMakeContextCurrent( context );
	if( alcGetError( device ) != ALC_NO_ERROR )
	{
		Log_Printf( "Failed to Make Context Current\n" );
		alcDestroyContext( context );
		context = 0;
		return;
	}
	
	
	Log_Printf( "[OpenAL] Context succesfully initialized.\n" );
    
    
	SND_BACKEND_OPENAL_GenerateChannels();
}



void SND_BACKEND_Play(int sndId){
    
    //currentChannel points to the first free channel
	ALuint source;
	
	
	if (!engine.soundEnabled)
		return;
	
	source = sources[sndId];
    
    ALint alBuffer = alMetadatas[sndId].alBuffer;

	
	
	//Uploading sounds to buffer
	
	//Trying stop as a bug fix (OpenAL drops sounds after a while for now reason.
	alSourceStop(source);
	
	//Log_Printf("Uploading sound %d, sampleRate=%ld\n",sndId,sound->metaData.sample_rate);
	alSourcef( source, AL_GAIN, 0.6f );
	alSourcei( source, AL_BUFFER, alBuffer );
	alSourcei( source, AL_LOOPING, 0 );
	alSourcei( source, AL_SOURCE_RELATIVE, AL_FALSE );
	alSourcePlay( source );
	
	//Log_Printf("playing sound %d on source %ud with soundBuffer %ud\n",sndId,source,sound->alBuffer);
	//if( alcGetError( device ) != ALC_NO_ERROR )
	//{
	//	Log_Printf("alcGetError()=%d\n",alcGetError( device ) );
	//}
	
	
	//currentChannel = (++currentChannel & (NUM_SOURCES-1));

    
    
}
