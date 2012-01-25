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
 *  material.h
 *  dEngine
 *
 *  Created by fabien sanglard on 24/09/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#ifndef DE_MATERIAL
#define DE_MATERIAL

#include "globals.h"
#include "texture.h"
#include "math.h"

#define TEXTURE_DIFFUSE 0
#define TEXTURE_BUMP 1
#define TEXTURE_SPECULAR 2

#define PROP_BUMP      0x01
#define PROP_SPEC      0x02
#define PROP_DIFF	   0x04
#define PROP_UNDEF1    0x08
#define PROP_UNDEF2    0x10
#define PROP_UNDEF3	   0x20
#define PROP_FOG	   0x40	
#define PROP_SHADOW    0x80


#define MAX_MATERIAL_NAME_LENGTH 256
typedef struct material_t
{
	char name[MAX_MATERIAL_NAME_LENGTH];
	
	float shininess;
	float specularColor[3];
	
	uchar prop;
	
	uchar hasAlpha;
	
	texture_t textures[3];
	
	
} material_t;

#define MATERIAL_QUALITY_LOW 0
#define MATERIAL_QUALITY_HIGH 1

void MAT_InitCacheSystem(void);
void MATLIB_SetMaterialQuality(uchar level);

material_t* MATLIB_Create(char* materialName);
material_t* MATLIB_Get(char* materialName);
void MATLIB_MakeAvailable(material_t* material);
void MAT_MarkMaterialResident(material_t* material);

void MATLIB_LoadLibrary(char* mtlPath);
void MATLIB_LoadLibraries(void);

// Tracking methods
void MATLIB_printProp(uchar prop);
void MATLIB_PrintCache();
#endif