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
 *  wavfile.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-06-06.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */


#ifndef __WAV_H__
#define __WAV_H__


// Structure used to describe a sound.
typedef struct 
{
	unsigned long	sample_rate;	// Sample rate in Hz
	unsigned long	channels;		// Number of Channels (0x01 = Mono, 0x02 = Stereo)
	unsigned long	sample_size;	// Bytes per sample	
	//		1 = 8 bit Mono
	//		2 = 8 bit Stereo or 16 bit Mono
	//		4 = 16 bit Stereo
	unsigned long	samples;
	
} soundInfo_t;



extern char LoadWavInfo( const char *filename, unsigned char **wav, soundInfo_t *info );

#endif /* __WAV_H__ */
