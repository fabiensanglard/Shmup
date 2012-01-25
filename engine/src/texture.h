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
 *  texture.h
 *  dEngine
 *
 *  Created by fabien sanglard on 16/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */


#ifndef FS_TEXTURE
#define FS_TEXTURE

#include "globals.h"
#include "filesystem.h"

#define TEXTURE_TYPE_UNKNOWN	0x0
#define TEXTURE_GL_RGB			0x1907
#define TEXTURE_GL_RGBA			0x1908


#define TEXT_MEM_LOC_DISK 0
#define TEXT_MEM_LOC_RAM 1
#define TEXT_MEM_LOC_VRAM 2

typedef struct  {
	uint textureId;
	char* path;
	
	uchar numMipmaps;
	ubyte** data;
	int* dataLength; //compressed texture need to provide their size
	
	filehandle_t* file;
	
	uint width;
	uint height;
	uint bpp;
	uint format;
	
	uchar cachable;
	uchar memLocation;
	uchar memStatic;
	
} texture_t;



void TEX_MakeAvailable(texture_t* texture);
void TEX_MakeStaticAvailable(texture_t* texture);
texture_t* TEX_GetTexture(char* path);
void TEX_UnloadTexture(texture_t* texture);

void TEXT_InitCacheSystem(void);
void TEXT_ClearTextureLibrary(void);

void TEXT_PrintCache(void);

#endif