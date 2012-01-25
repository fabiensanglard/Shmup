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
 *  fx.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-02-24.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */



#ifndef DE_FXS
#define DE_FXS

#include "math.h"
#include "enemy.h"
#include "texture.h"

#define IMPACT_TYPE_YELLOW 0
#define IMPACT_TYPE_BLUE 1

typedef struct explosion_t
{
	int timeCounter;
	short ss_boundaries[4];
	
	short ss_MaxBoundaries[4];
	short ss_MinBoundaries[4];
	
	short ss_Diff[4];
	
	struct explosion_t* next;
	struct explosion_t* prev;
	
	uchar type;
	
} explosion_t;

#define PARTICULE_COLOR_YELLOW 0
#define PARTICULE_COLOR_WHITE 1
#define PARTICULE_COLOR_RED 2
#define PARTICULE_COLOR_BLUE 3
typedef struct particule_t
{
	int timeCounter;
	
	vec2short_t ss_startBorders[4];
	vec2short_t ss_endBorders[4];
	vec2short_t ss_diff[4];
	
	struct particule_t* next;
	struct particule_t* prev;
	
	float type;
	
	float colorType;
	
} particule_t;

typedef struct xf_sprite_t
{
	vec2short_t pos;
	vec2short_t text;
	ubyte color[4];
	
} xf_sprite_t;

typedef struct xf_colorless_sprite_t
{
	vec2short_t pos;
	vec2short_t text;	
} xf_colorless_sprite_t;




void FX_InitMem(void);

//EXPLOSIONS
#define MAX_NUM_EXPLOSIONS MAX_NUM_ENEMIES
#define MAX_NUM_EXPLOSION_VERTICES (MAX_NUM_EXPLOSIONS*4)
#define MAX_NUM_EXPLOSIONS_INDICES (MAX_NUM_EXPLOSIONS* 6)


extern xf_sprite_t explosionVertices[MAX_NUM_EXPLOSION_VERTICES]; 
extern int numExplosionVertices;

extern int numExplosionIndices;
extern unsigned short explosionIndices[MAX_NUM_EXPLOSIONS_INDICES];
extern texture_t explosionTexture;

void FX_UpdateExplosions(void);
void FX_PrepareExplosionsSprites(void);
explosion_t* FX_GetExplosion(vec2_t ss_position, uchar type,float sizeFactor, float mouvementY);
explosion_t* FX_GetFirstExplosion(void);
void FX_ReleaseExplosion(explosion_t* explosion);


//PARTICULES
#define MAX_NUM_PARTICULES (MAX_NUM_EXPLOSIONS*8)
#define MAX_NUM_PARTICULES_VERTICES (MAX_NUM_PARTICULES * 4)
#define MAX_NUM_PARTICULE_INDICES (MAX_NUM_PARTICULES * 6)

extern xf_sprite_t particuleVertices[MAX_NUM_PARTICULES_VERTICES]; 
extern int numParticuleVertices;

extern int numParticulesIndices;
extern unsigned short particuleIndices[MAX_NUM_PARTICULE_INDICES];


void FX_UpdateParticules(void);
void FX_PrepareParticulesSprites(void);

#define PARTICULE_TYPE_EXPLOSION 0
#define PARTICULE_TYPE_IMPLOSION 1
#define PARTICULE_SIZE_GLOBAL 0.8f
#define PARTICULE_TRAVEL_DIST 0.55
#define PARTICULE_DEFAULT_STRECH 3
particule_t* FX_GetParticule(vec2_t ss_position, vec2_t direction, float size, float travelDistance,uchar type, float colorType, int strech);
particule_t* FX_GetFirstParticule(void);
void FX_ReleaseParticule(particule_t* particule);


//SMOKE
#define MAX_NUM_SMOKE MAX_NUM_ENEMIES
#define MAX_NUM_SMOKE_VERTICES (MAX_NUM_SMOKE*4)
#define MAX_NUM_SMOKE_INDICES (MAX_NUM_SMOKE* 6)

extern xf_colorless_sprite_t smokeVertices[MAX_NUM_SMOKE_VERTICES];


extern ushort smokeIndices[MAX_NUM_SMOKE_INDICES];
extern int numSmokeIndices;

extern texture_t smokeTexture;

typedef struct smoke_t
{
	int timeCounter;
	int ttl;
	

	
	ushort vertexStart;
	//xf_sprite_t* vertices;
	
	//vec2short_t ss_startBorders[4];
	//vec2short_t ss_endBorders[4];
	vec2short_t text_coo[4];
	
	struct smoke_t* next;
	struct smoke_t* prev;
	
} smoke_t;

void FX_UpdateSmoke(void);
void FX_PrepareSmokeSprites(void);
smoke_t* FX_GetSmoke(vec2_t ss_position,float ss_sizeX, float ss_sizeY);
smoke_t* FX_GetFirstSmoke(void);
void FX_ReleaseSmoke(smoke_t* smoke);

#endif