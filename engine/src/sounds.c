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
 *  sounds.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-06-06.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include "sounds.h"
#include <limits.h>
#include "dEngine.h"
#include "timer.h"

#ifdef WIN32
	#include "al.h"
	#include "alc.h"
#else
	#include "OpenAL/al.h"
	#include "OpenAL/alc.h"
#endif

sound_t sounds[8];

#define NUM_SOURCES 8
ALuint sources[NUM_SOURCES];

ALCcontext* context;
ALCdevice* device;

void SND_Load(char* filename,sound_t* sound)
{
	
	if (!LoadWavInfo(filename, &sound->data, &sound->metaData ))
			printf("[SND_Load] Unable to load sound: '%s'.\n",filename);
	
	printf("[SND_Load] Loaded sound: %s sample_rate=%ld ",filename,sound->metaData.sample_rate);
	
	// Calculate buffer size
	sound->size = sound->metaData.samples * sound->metaData.sample_size * sound->metaData.channels;
	
	if( sound->metaData.sample_size == 2 )
	{
		if( sound->metaData.channels == 2 )
		{
			sound->format = AL_FORMAT_STEREO16;
			printf("format=AL_FORMAT_STEREO16.\n");
		}
		else
		{
			sound->format = AL_FORMAT_MONO16;
			printf("format=AL_FORMAT_MONO16.\n");			
		}
	}
	else if( sound->metaData.sample_size == 1 )
	{
		if( sound->metaData.channels == 2 )
		{
			sound->format = AL_FORMAT_STEREO8;
			printf("format=AL_FORMAT_STEREO8.\n");			
		}
		else
		{
			sound->format = AL_FORMAT_MONO8;
			printf("format=AL_FORMAT_MONO8.\n");
		}
	}
	//else
	//	printf("format=UNKNOWN.\n");
	sound->lastTimePlayed = INT_MIN;
	
	
	alGenBuffers(1, &sound->alBuffer);
	alBufferData(sound->alBuffer, sound->format, sound->data, sound->size, sound->metaData.sample_rate );
	
#ifndef GENERATE_VIDEO
	free(sound->data);
	sound->data=0;
#else
	printf("Warning, not freeing WAV after openAL upload.\n");
#endif
}


void SND_GenerateChannels(void )
{
	int i;
	
	for (i=0; i < NUM_SOURCES; i++) 
	{
		alGenSources(1, &sources[i]);
	}
}

void SND_LoadsSoundLibrary(void )
{
	SND_Load("data/sfx/plasma.wav", &sounds[SND_PLASMA]);
	SND_Load("data/sfx/explosionShort.wav", &sounds[SND_EXPLOSION]);
	SND_Load("data/sfx/ghostLauch.wav", &sounds[SND_GHOST_LAUNCH]);
	SND_Load("data/sfx/enemy_shot.wav", &sounds[SND_ENEMY_SHOT]);
}


char *deviceList;
char *sound_devices[ 12 ];
ushort numSoundDevices, numDefaultSoundDevice;
void SND_GetDeviceList( void )
{
	char deviceName[ 256 ];
	
	//my_strlcpy( deviceName, s_device->string, sizeof( deviceName ) );
	if( alcIsExtensionPresent( NULL,"ALC_ENUMERATION_EXT") == AL_TRUE ) 
	{	
		// try out enumeration extension
		deviceList = (char *)alcGetString( NULL, ALC_DEVICE_SPECIFIER );
		
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

int SND_Init(void)
{
	
	printf("[SND_Init] Initalizing sound system...\n");
	SND_GetDeviceList();
	
	device = alcOpenDevice( NULL  );
	if( device == NULL )
	{
		printf( "Failed to Initialize OpenAL\n" );
		alcDestroyContext( context );
		context = 0;
		return 0;
	}
	
	// Create context(s)
	context = alcCreateContext( device, NULL );
	if( context == NULL )
	{
		printf( "Failed to initialize OpenAL\n" );
		alcDestroyContext( context );
		context = 0;
		return 0;
	}
	
	
	
	// Set active context
	alcGetError( device );
	alcMakeContextCurrent( context );
	if( alcGetError( device ) != ALC_NO_ERROR )
	{
		printf( "Failed to Make Context Current\n" );
		alcDestroyContext( context );
		context = 0;
		return 0;
	}
	
	
	printf( "[OpenAL] Context succesfully initialized.\n" );
		

	SND_GenerateChannels();
	
	SND_LoadsSoundLibrary();
		
	return 1;

}


//char currentChannel=0;
void SND_PlaySound(int sndId)
{
	//currentChannel points to the first free channel
	ALuint source;
	sound_t* sound;
	
	if (!engine.soundEnabled)
		return;
	
	source = sources[sndId];
	sound = &sounds[sndId];
	
	sound->lastTimePlayed = simulationTime;
	//Uploading sounds to buffer
	
	//Trying stop as a bug fix (OpenAL drops sounds after a while for now reason.
	alSourceStop(source);
	
	//printf("Uploading sound %d, sampleRate=%ld\n",sndId,sound->metaData.sample_rate);
	alSourcef( source, AL_GAIN, 0.6f );
	alSourcei( source, AL_BUFFER, sound->alBuffer );
	alSourcei( source, AL_LOOPING, 0 );
	alSourcei( source, AL_SOURCE_RELATIVE, AL_FALSE );
	alSourcePlay( source );
	
	//printf("playing sound %d on source %ud with soundBuffer %ud\n",sndId,source,sound->alBuffer);
	//if( alcGetError( device ) != ALC_NO_ERROR )
	//{
	//	printf("alcGetError()=%d\n",alcGetError( device ) );
	//}
	
	
	//currentChannel = (++currentChannel & (NUM_SOURCES-1));
}


uchar* audioTrack=0;
unsigned int sizeAudioTrack ;

void SND_UpdateRecord(void)
{
#ifdef GENERATE_VIDEO
	
	int numFrameToWrite= 0;
	int i;
	short sndValue;
	char numActiveChannel=0;
	

	// DEPRECATED, we now run timestep=16 and timestep = 17 in order to achieve (timestep 16.6666667ms)
	// DEPRECATED, we now run timestep=16 and timestep = 17 in order to achieve (timestep 16.6666667ms)
	// We are doing a few weird things here: The game simulate time at 16ms timestep but the iPhone screen runs at 60Hz (timestep 16.6666667ms)
	// This is not an issue during the game but it is when generating a soundtrack frame by frame.
	// Quicktime can only import at 60fps so we should not assume the video time with  timediff or simulationTime to generate the soundtrack.
	// We need to generate sounds for the realtime of the frame not the simulation time.
	// DEPRECATED, we now run timestep=16 and timestep = 17 in order to achieve (timestep 16.6666667ms)
	// DEPRECATED, we now run timestep=16 and timestep = 17 in order to achieve (timestep 16.6666667ms)
	// DEPRECATED, we now run timestep=16 and timestep = 17 in order to achieve (timestep 16.6666667ms)
	
	numFrameToWrite = timediff * 22050/1000; 

	
	if (audioTrack == 0)
	{
		audioTrack = malloc(22050*240 * sizeof(uchar));
		memset(audioTrack,127,22050*240*sizeof(uchar));
	}
	//We need to generate the next frames for the amount timediff milliseconds.

	//printf("%ld.\n",sounds[SND_PLASMA].metaData.samples);

	/*
	if (sounds[SND_PLASMA].lastTimePlayed > 0)
	printf("(simulationTime - sounds[SND_PLASMA].lastTimePlayed) * (22050.0/1000)=%d\n",(simulationTime - sounds[SND_PLASMA].lastTimePlayed) * (22050/1000));
	*/
	
	for (i=0; i < numFrameToWrite; i++) 
	{
		sndValue = 0;
		numActiveChannel=0;
		// Generate sound's frame position and mix them
		if (((simulationTime - sounds[SND_PLASMA].lastTimePlayed) * (22050.0/1000)+i) < sounds[SND_PLASMA].metaData.samples) 
		{
			sndValue = sounds[SND_PLASMA].data[((simulationTime - sounds[SND_PLASMA].lastTimePlayed)*22050/1000)+i];
			numActiveChannel++;
			//printf("Writing plasma [%i]\n",sounds[SND_PLASMA].data[((simulationTime - sounds[SND_PLASMA].lastTimePlayed)*22050/1000)+i);
		}

		if (((simulationTime - sounds[SND_EXPLOSION].lastTimePlayed) * (22050.0/1000)+i) < sounds[SND_EXPLOSION].metaData.samples)
		{
			sndValue += sounds[SND_EXPLOSION].data[((simulationTime - sounds[SND_EXPLOSION].lastTimePlayed)*22050/1000)+i];
			numActiveChannel++;
			//printf("Writing plasma [ %d/%ld]\n",((simulationTime - sounds[SND_PLASMA].lastTimePlayed)*22050/1000)+i,sounds[SND_PLASMA].metaData.samples);
		}
		
		if (((simulationTime - sounds[SND_GHOST_LAUNCH].lastTimePlayed) * (22050.0/1000)+i) < sounds[SND_GHOST_LAUNCH].metaData.samples)
		{
			sndValue += sounds[SND_GHOST_LAUNCH].data[((simulationTime - sounds[SND_GHOST_LAUNCH].lastTimePlayed)*22050/1000)+i];
			numActiveChannel++;
			//printf("Writing plasma [ %d/%ld]\n",((simulationTime - sounds[SND_PLASMA].lastTimePlayed)*22050/1000)+i,sounds[SND_PLASMA].metaData.samples);
		}
		
		if (((simulationTime - sounds[SND_ENEMY_SHOT].lastTimePlayed) * (22050.0/1000)+i) < sounds[SND_ENEMY_SHOT].metaData.samples)
		{
			sndValue += sounds[SND_ENEMY_SHOT].data[((simulationTime - sounds[SND_ENEMY_SHOT].lastTimePlayed)*22050/1000)+i];
			numActiveChannel++;
			//printf("Writing plasma [ %d/%ld]\n",((simulationTime - sounds[SND_PLASMA].lastTimePlayed)*22050/1000)+i,sounds[SND_PLASMA].metaData.samples);
		}
		
		if (numActiveChannel)
			sndValue /= numActiveChannel;												   
		else {
			sndValue = 127;
		}

		//printf("Writing %uc\n",sndValue);
		audioTrack[simulationTime * 22050/1000 +i] = sndValue ;
		
	}
	sizeAudioTrack+=i;
	
	
#endif	
}

#define WAVE_FORMAT_PCM			0x0001		
#define WAVE_FORMAT_IEEE_FLOAT	0x0003	
#define WAVE_FORMAT_ALAW		0x0006	
#define WAVE_FORMAT_MULAW		0x0007
#define WAVE_FORMAT_EXTENSIBLE	0xFFFE

typedef struct master_riff_chnk_t
{
	char			ckID[4];
	unsigned int	cksize;
	char			WAVEID[4];
} master_riff_chnk_t;

typedef struct fmt_chunk_t
{
	char			ckID[4];
	unsigned int	cksize;
	unsigned short	wFormatTag;//	 2	 WAVE_FORMAT_PCM
	unsigned short	nChannels;//	 2	Nc
	unsigned int	nSamplesPerSec;//	 4	F
	unsigned int	nAvgBytesPerSec;//	 4	F * M * Nc
	unsigned short	nBlockAlign	;// 2	M * Nc
	unsigned short	wBitsPerSample;//	 2	rounds up to 8 * M
	
} fmt_chunk_t;

typedef struct basic_chunk_t
{
	char			ckID[4];
	unsigned int	cksize;
} basic_chunk_t;

typedef struct wave_file_t
{
	master_riff_chnk_t riff;
	fmt_chunk_t fmt;
	basic_chunk_t dataChunk;
} wave_file_t ;

#define SOUND_TRACK_LOCATION "/Users/fabiensanglard/Pictures/dEngine/audioTrack.wav"

void SND_FinalizeRecord(void)
{
#ifdef GENERATE_VIDEO	
	wave_file_t waveFile;
	FILE* f;
	
	
	
	memset(&waveFile,0,sizeof(waveFile));
	
	waveFile.riff.cksize =  4 + 24 + (8 + sizeAudioTrack);  //4(dataChunk header) + 24(fmt header) + 8 (riff header) + data payload
	memcpy(waveFile.riff.ckID,"RIFF",4);
    memcpy(waveFile.riff.WAVEID,"WAVE",4);
	
	waveFile.dataChunk.cksize = sizeAudioTrack;
	memcpy(waveFile.dataChunk.ckID,"data",4);
	
	memcpy(waveFile.fmt.ckID,"fmt ",4);
	waveFile.fmt.cksize = 16;
	waveFile.fmt.wFormatTag = WAVE_FORMAT_PCM;
	waveFile.fmt.nChannels = 1;
	waveFile.fmt.nSamplesPerSec = 22050;
	waveFile.fmt.wBitsPerSample = 8;
	waveFile.fmt.nBlockAlign = waveFile.fmt.wBitsPerSample/8 * waveFile.fmt.nChannels;
	waveFile.fmt.nAvgBytesPerSec = waveFile.fmt.nSamplesPerSec * waveFile.fmt.wBitsPerSample/8 * waveFile.fmt.nChannels;

	
	
	
	
	//Write file on disk
	f = fopen(SOUND_TRACK_LOCATION, "wb");
	
	if (!f)
	{
		printf("Error creating file %s .\n",SOUND_TRACK_LOCATION);
	}
	
	fwrite(&waveFile, 1, sizeof(wave_file_t), f);
	//fwrite(&waveFile.fmt, 1, sizeof(fmt_chunk_t), f);
	fwrite(audioTrack, 1, sizeAudioTrack, f);
	fclose(f);
	
	//free(audioTrack);
	//audioTrack=0;
#endif
}


