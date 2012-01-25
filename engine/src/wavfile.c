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
 *  wavfile.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-06-06.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include "wavfile.h"
#include "filesystem.h"
#include <string.h>



unsigned char *iff_pdata;
unsigned char *iff_end;
unsigned char *iff_last_chunk;
unsigned char *iff_data;
int	iff_chunk_len;



short Wav_GetLittleShort( void )
{
	short val = 0;
	
	val = *iff_pdata;
	val += (*(iff_pdata + 1) << 8);
	
	iff_pdata += 2;
	
	return val;
}


int Wav_GetLittleLong( void )
{
	int val = 0;
	
	val =  *iff_pdata;
	val += (*(iff_pdata + 1) << 8);
	val += (*(iff_pdata + 2) << 16);
	val += (*(iff_pdata + 3) << 24);
	
	iff_pdata += 4;
	
	return val;
}


void Wav_FindNextChunk( const char *name )
{
	while( 1 )
	{
		iff_pdata = iff_last_chunk;
		
		if( iff_pdata >= iff_end )
		{
			// Didn't find the chunk
			iff_pdata = 0;
			return;
		}
		
		iff_pdata += 4;
		iff_chunk_len = Wav_GetLittleLong();
		if( iff_chunk_len < 0 )
		{
			iff_pdata = 0;
			return;
		}
		
		iff_pdata -= 8;
		iff_last_chunk = iff_pdata + 8 + ((iff_chunk_len + 1) & ~1);
		if( ! strncmp((const char *)iff_pdata, name, 4) )
		{
			return;
		}
	}
}


void Wav_FindChunk( const char *name )
{
	iff_last_chunk = iff_data;
	
	Wav_FindNextChunk( name );
}

void DumpChunks( void )
{
	char str[ 5 ];
	
	str[ 4 ] = 0;
	iff_pdata = iff_data;
	do
	{
		memcpy( str, iff_pdata, 4 );
		iff_pdata += 4;
		iff_chunk_len = Wav_GetLittleLong();
		printf( "0x%x : %s (%d)\n", (int)(iff_pdata - 4), str, iff_chunk_len );
		iff_pdata += (iff_chunk_len + 1) & ~1;
		
	} while( iff_pdata < iff_end );
	
}


/*
 -----------------------------------------------------------------------------
 Function: LoadWavInfo -Load wav file.
 
 Parameters: filename -[in] Name of wav file to load.
 wav -[out] wav data.
 info -[out] wav sound info.
 
 Returns: True if file loaded, otherwise false.
 
 Notes: Caller is responsible for freeing wav data by calling Z_Free.
 
 -----------------------------------------------------------------------------
 */
char LoadWavInfo( const char *filename, unsigned char **wav, soundInfo_t *info )
{
	filehandle_t *hFile;
	unsigned char 	*data;
	unsigned long	wavlength;
	
	hFile = FS_OpenFile( filename, "r");
	if( ! hFile )
		return 0;
	
	
	data = (PW8)FS_GetLoadedFilePointer( hFile, SEEK_SET );
	wavlength = FS_GetFileSize( hFile );
	
	iff_data = data;
	iff_end = data + wavlength;
	
	// look for RIFF signature
	Wav_FindChunk( "RIFF" );
	if( ! (iff_pdata && ! strncmp( (const char *)iff_pdata + 8, "WAVE", 4 ) ) )
	{
		printf( "[LoadWavInfo]: Missing RIFF/WAVE chunks (%s)\n", filename );
		FS_CloseFile( hFile );
		
		return 0;
	}
	
	// Get "fmt " chunk
	iff_data = iff_pdata + 12;
	
	Wav_FindChunk("fmt ");
	if( ! iff_pdata )
	{
		printf( "[LoadWavInfo]: Missing fmt chunk (%s)\n", filename );
		FS_CloseFile( hFile );
		
		return 0;
	}
	
	iff_pdata += 8;
	
	if( Wav_GetLittleShort() != 1 )
	{
		printf( "[LoadWavInfo]: Microsoft PCM format only (%s)\n", filename );
		FS_CloseFile( hFile );
		
		return 0;
	}
	
	info->channels = Wav_GetLittleShort();
	info->sample_rate = Wav_GetLittleLong();
	
	iff_pdata += 4;
	
	info->sample_size = Wav_GetLittleShort(); // Bytes Per Sample
	
	if (info->sample_size != 1 && info->sample_size != 2)
	{
		printf( "[LoadWavInfo]: only 8 and 16 bit WAV files supported (%s)\n", filename );
		FS_CloseFile( hFile );
		
		return 0;
	}
	
	iff_pdata += 2;
	
	
	// Find data chunk
	Wav_FindChunk( "data" );
	if( ! iff_pdata )
	{
		printf( "[LoadWavInfo]: missing 'data' chunk (%s)\n", filename );
		FS_CloseFile( hFile );
		
		return 0;
	}
	
	iff_pdata += 4;
	info->samples = Wav_GetLittleLong() / info->sample_size;
	
	if( info->samples <= 0 )
	{
		printf( "[LoadWavInfo]: file with 0 samples (%s)\n", filename );
		FS_CloseFile( hFile );
		
		return 0;
	}
	
	// Load the data
	*wav = malloc( info->samples * info->sample_size );
	memcpy( *wav, data + (iff_pdata - data), info->samples * info->sample_size );
	
	FS_CloseFile( hFile );
	
	return 1;
}

