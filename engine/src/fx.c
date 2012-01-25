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
 *  fx.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-02-24.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include "fx.h"
#include "enemy.h"
#include "texture.h"
#include "timer.h"
#include "renderer.h"
#include <limits.h>
#include "enemy.h"

//EXPLOSIONS VARS
texture_t explosionTexture;


explosion_t rootExplosion;
explosion_t explosions[MAX_NUM_EXPLOSIONS];
ushort numFreeExplosions;
explosion_t* freeExplosions[MAX_NUM_EXPLOSIONS];

//RENDITION
xf_sprite_t explosionVertices[MAX_NUM_EXPLOSION_VERTICES]; 
int numExplosionVertices=0;

int numExplosionIndices=0;
unsigned short explosionIndices[MAX_NUM_EXPLOSIONS_INDICES];





particule_t rootParticule;
particule_t particules[MAX_NUM_PARTICULES];
ushort numFreeParticules;
particule_t* freeParticules[MAX_NUM_PARTICULES];

xf_sprite_t particuleVertices[MAX_NUM_PARTICULES_VERTICES]; 
int numParticuleVertices;
int numParticulesIndices;
unsigned short particuleIndices[MAX_NUM_PARTICULE_INDICES];



xf_colorless_sprite_t smokeVertices[MAX_NUM_SMOKE_VERTICES];

ushort smokeIndices[MAX_NUM_SMOKE_INDICES];
int numSmokeIndices;
smoke_t rootSmoke;
texture_t smokeTexture;
smoke_t smokes[MAX_NUM_SMOKE];
ushort numFreeSmokes;
smoke_t* freeSmokes[MAX_NUM_SMOKE];


void FX_InitMem(void)
{
	int i;
	int numSmokeVertices=0;
	xf_sprite_t* sprite;
	
	
	// INIT EXPLOSIONS
	TEX_MakeStaticAvailable(&explosionTexture);
	//renderer.UpLoadTextureToGpu(&explosionTexture);
	
	for (i=0; i < MAX_NUM_EXPLOSIONS; i++) 
	{
		freeExplosions[i] = &explosions[i];
	}
	
	numFreeExplosions = MAX_NUM_EXPLOSIONS;
	
	
	rootExplosion.next = NULL;
	rootExplosion.prev = NULL;
	
	
	//Pre-generate static indices and vertices info.
	for (i=0 ; i < MAX_NUM_EXPLOSION_VERTICES; i++) 
	{
		sprite = &explosionVertices[i];
		sprite->color[R] = 255;
		sprite->color[G] = 255;
		sprite->color[B] = 255;
	}
	
	numExplosionVertices=0 ;
	for (i=0; i < MAX_NUM_EXPLOSIONS_INDICES; i+=6, numExplosionVertices+=4 ) 
	{
		explosionIndices[i+0] = numExplosionVertices+0;
		explosionIndices[i+1] = numExplosionVertices+1;
		explosionIndices[i+2] = numExplosionVertices+3;
		explosionIndices[i+3] = numExplosionVertices+3;
		explosionIndices[i+4] = numExplosionVertices+1;
		explosionIndices[i+5] = numExplosionVertices+2;
	}
	numExplosionVertices=0;
	
	
	
	
	
	
	
	
	
	
	
	//INIT PARTICULES

	for (i=0; i < MAX_NUM_PARTICULES; i++) 
		freeParticules[i] = &particules[i];
	
	numFreeParticules = MAX_NUM_PARTICULES;
	
	rootParticule.next = NULL;
	rootParticule.prev = NULL;
	
	sprite = particuleVertices;
	for (i=0 ; i < MAX_NUM_PARTICULES_VERTICES; i++) 
	{

		sprite->color[R] = 255;
		sprite->color[G] = 255;
		sprite->color[B] = 255;
		
		//Also write texture coordinate
//		uchar boolean1 = ;
//		uchar boolean2 = ;
//		uchar boolean = boolean1 ^ boolean2;
		//sprite->text[U] = 49/128.0f * SHRT_MAX + 15/128.0f*SHRT_MAX * (((i%4)&1) == ((i%4)&2)>>1);
		sprite->text[V] = 16/128.0f* SHRT_MAX * ((i&2)>>1 ^ i&1);//((i&2) ^ (i&1));
		
		//sprite->text[X] = SHRT_MAX * (((i%4)&1) == ((i%4)&2)>>1);
		//sprite->text[Y] = SHRT_MAX * (((i%4)&2) == 2);
		
		
		/*
			0  3
			1  2
		 */
		
		//0
		//particuleSpriteVertice->text[X] = SHRT_MAX;
		//particuleSpriteVertice->text[Y] = 0;
		
		//1
		//particuleSpriteVertice->text[X] = 0;
		//particuleSpriteVertice->text[Y] = 0;

		//2
		//particuleSpriteVertice->text[X] = 0;
		//particuleSpriteVertice->text[Y] = SHRT_MAX;

		//3
		//particuleSpriteVertice->text[X] = SHRT_MAX;
		//particuleSpriteVertice->text[Y] = SHRT_MAX;
		

		
		sprite++;
	}
	
	
	numParticuleVertices=0;
	for (i=0; i < MAX_NUM_PARTICULE_INDICES; i+=6, numParticuleVertices+=4 ) 
	{
		particuleIndices[i+0] = numParticuleVertices+0;
		particuleIndices[i+1] = numParticuleVertices+1;
		particuleIndices[i+2] = numParticuleVertices+3;
		particuleIndices[i+3] = numParticuleVertices+3;
		particuleIndices[i+4] = numParticuleVertices+1;
		particuleIndices[i+5] = numParticuleVertices+2;
		
	}
	numParticuleVertices=0;
	
	
	
	
	//Smoke
	//smokeTexture.path = calloc(1, strlen("data/texturesPVR/sprites/explosion512.pvr")+1);
	//strcpy(smokeTexture.path,"data/texturesPVR/sprites/explosion512.pvr");
	TEX_MakeStaticAvailable(&smokeTexture);
	//renderer.UpLoadTextureToGpu(&smokeTexture);
	
	for (numFreeSmokes=0; numFreeSmokes < MAX_NUM_SMOKE; numFreeSmokes++) 
	{
		freeSmokes[numFreeSmokes] = &smokes[numFreeSmokes];
		smokes[numFreeSmokes].vertexStart = numFreeSmokes*4;
		//smokes[numFreeSmokes].vertices = &smokeVertices[numFreeSmokes*4];
	}
	numFreeSmokes = MAX_NUM_SMOKE;
	
	rootSmoke.next = NULL;
	rootSmoke.prev = NULL;
	
	/*
	sprite = smokeVertices;
	for (i=0 ; i < MAX_NUM_SMOKE_VERTICES; i++,sprite++) 
	{
		sprite->color[R] = 255;
		sprite->color[G] = 255;
		sprite->color[B] = 255;
		sprite->color[A] = 255;
	}
	*/
	
	for (i=0; i < MAX_NUM_SMOKE_INDICES; i+=6, numSmokeVertices+=4 ) 
	{
		smokeIndices[i+0] = numSmokeVertices+0;
		smokeIndices[i+1] = numSmokeVertices+1;
		smokeIndices[i+2] = numSmokeVertices+3;
		smokeIndices[i+3] = numSmokeVertices+3;
		smokeIndices[i+4] = numSmokeVertices+1;
		smokeIndices[i+5] = numSmokeVertices+2;
		
	}
	numSmokeVertices=0;
	
	
}

explosion_t* FX_GetFirstExplosion(void)
{
	return rootExplosion.next;
}

#define EXPLOSION_START_SIZE 0.05f
#define EXPLOSION_END_SIZE 0.17f
explosion_t nullExplosion;
explosion_t* FX_GetExplosion(vec2_t ss_position, uchar type,float sizeFactor, float mouvementY)
{
	explosion_t* explosion;
	
	if (numFreeExplosions == 0)
		return &nullExplosion;
	
	numFreeExplosions--;
	explosion = freeExplosions[numFreeExplosions];
	
	explosion->ss_MaxBoundaries[UP] 	= (mouvementY + ss_position[Y] + sizeFactor*EXPLOSION_END_SIZE)*SS_H;
	explosion->ss_MaxBoundaries[DOWN] 	= (mouvementY + ss_position[Y] - sizeFactor*EXPLOSION_END_SIZE)*SS_H;
	explosion->ss_MaxBoundaries[LEFT]	= (ss_position[X] - sizeFactor*EXPLOSION_END_SIZE)*SS_H;
	explosion->ss_MaxBoundaries[RIGHT]	= (ss_position[X] + sizeFactor*EXPLOSION_END_SIZE)*SS_H;
	
	explosion->ss_MinBoundaries[UP]		= (ss_position[Y] + sizeFactor*EXPLOSION_START_SIZE)*SS_H;
	explosion->ss_MinBoundaries[DOWN]	= (ss_position[Y] - sizeFactor*EXPLOSION_START_SIZE)*SS_H;
	explosion->ss_MinBoundaries[LEFT]	= (ss_position[X] - sizeFactor*EXPLOSION_START_SIZE)*SS_H;
	explosion->ss_MinBoundaries[RIGHT]	= (ss_position[X] + sizeFactor*EXPLOSION_START_SIZE)*SS_H;
	

	explosion->ss_Diff[UP]		= explosion->ss_MaxBoundaries[UP]		- explosion->ss_MinBoundaries[UP] ;
	explosion->ss_Diff[DOWN]	= explosion->ss_MaxBoundaries[DOWN]		- explosion->ss_MinBoundaries[DOWN] ;
	explosion->ss_Diff[LEFT]	= explosion->ss_MaxBoundaries[LEFT]		- explosion->ss_MinBoundaries[LEFT] ;
	explosion->ss_Diff[RIGHT]	= explosion->ss_MaxBoundaries[RIGHT]	- explosion->ss_MinBoundaries[RIGHT] ;
	
	
	explosion->timeCounter = 0;
	
	//Insert after rootEnemy
	explosion->next = rootExplosion.next;
	
	if (explosion->next != NULL)
		explosion->next->prev =explosion;
	
	explosion->prev = &rootExplosion;
	rootExplosion.next = explosion;
	
	explosion->type = type;
	
	return explosion;
	
}

void FX_ReleaseExplosion(explosion_t* explosion)
{
	freeExplosions[numFreeExplosions] = explosion;
	numFreeExplosions++;
	
	if (explosion->prev != NULL)
		explosion->prev->next = explosion->next;
	
	if (explosion->next != NULL)
		explosion->next->prev = explosion->prev;
	
}

#define NUM_EXPLOSIONS_STAGES 3


#define EXPLOSION_TTL 400
//#define EXPLOSION_TTL 370
#define EXPLOSION_FLASH_TIME (100)
#define EXPLOSION_WAVE_TIME (EXPLOSION_TTL-EXPLOSION_FLASH_TIME)
#define EXPLOSION_FLASH_ALPHA 220
#define EXPLOSION_STARTING_ALPHA 255
#define EXPLOSION_ENDING_ALPHA 50


void FX_UpdateExplosions(void)
{
	explosion_t* explosion;
	xf_sprite_t* spriteVertice;
	int expStage=0;
	ubyte expAlpha=255;
	float interPolationF;
	explosion = FX_GetFirstExplosion();
	
	//numSpritesVertices = 0;
	numExplosionIndices = 0;
	
	
	spriteVertice = explosionVertices;
	
	while (explosion != NULL)
	{
		
		//UPDATE
		if (explosion->timeCounter >= EXPLOSION_TTL)
		{
			FX_ReleaseExplosion(explosion);
			explosion = explosion->next;
			continue;
		}
		
		
		
		if (explosion->timeCounter <= EXPLOSION_FLASH_TIME )
		{
			interPolationF = explosion->timeCounter/(float)EXPLOSION_FLASH_TIME;
			//Update current Explosion
			explosion->ss_boundaries[UP]	= (explosion->ss_MaxBoundaries[UP]	- interPolationF * explosion->ss_Diff[UP]);
			explosion->ss_boundaries[DOWN] = (explosion->ss_MaxBoundaries[DOWN]	- interPolationF * explosion->ss_Diff[DOWN]);
			explosion->ss_boundaries[RIGHT]= (explosion->ss_MaxBoundaries[RIGHT]	- interPolationF * explosion->ss_Diff[RIGHT]);
			explosion->ss_boundaries[LEFT] = (explosion->ss_MaxBoundaries[LEFT]	- interPolationF * explosion->ss_Diff[LEFT]);
			
			expStage = 0;
			expAlpha = EXPLOSION_FLASH_ALPHA;
				
		}
		
		else 
		{	
			//interPolationF = (explosion->timeCounter - EXPLOSION_FLASH_TIME)/(float)EXPLOSION_WAVE_TIME;
			interPolationF = (explosion->timeCounter- EXPLOSION_FLASH_TIME)/(float)(EXPLOSION_TTL- EXPLOSION_FLASH_TIME);
			//interPolationF = explosion->timeCounter/(float)EXPLOSION_TTL;
			//interPolationF = (1-cosf(interPolationF*M_PI))/2;
			
			//Update current Explosion
			explosion->ss_boundaries[UP]	= explosion->ss_MinBoundaries[UP]	+ interPolationF * explosion->ss_Diff[UP];
			explosion->ss_boundaries[DOWN] = explosion->ss_MinBoundaries[DOWN]	+ interPolationF * explosion->ss_Diff[DOWN];
			explosion->ss_boundaries[RIGHT]= explosion->ss_MinBoundaries[RIGHT] + interPolationF * explosion->ss_Diff[RIGHT];
			explosion->ss_boundaries[LEFT] = explosion->ss_MinBoundaries[LEFT]	+ interPolationF * explosion->ss_Diff[LEFT];
			
			expStage = interPolationF * 3.0f ;//explosion->type  ;
			expStage += 1;
			
			expAlpha = EXPLOSION_STARTING_ALPHA + interPolationF * (EXPLOSION_ENDING_ALPHA - EXPLOSION_STARTING_ALPHA) ;
			//if (expAlpha < 0)
			//	expAlpha = 0;
			
		}
		

		
		explosion->timeCounter += timediff;
		
		
		//RENDITION
		//Generate vertices and indices
		
		
		/*
			0  3 
			1  2	 
		 */
		
		
		spriteVertice->pos[X] = explosion->ss_boundaries[LEFT];
		spriteVertice->pos[Y] = explosion->ss_boundaries[UP];
		spriteVertice->text[U] = expStage* SHRT_MAX/4;
		spriteVertice->text[V] = explosion->type * SHRT_MAX/4;
		spriteVertice->color[A] = expAlpha;
		spriteVertice++;
		

		spriteVertice->pos[X] = explosion->ss_boundaries[LEFT];
		spriteVertice->pos[Y] = explosion->ss_boundaries[DOWN];
		spriteVertice->text[U] = expStage* SHRT_MAX/4;
		spriteVertice->text[V] = explosion->type* SHRT_MAX/4 + SHRT_MAX*1.f/4;
		spriteVertice->color[A] = expAlpha;
		spriteVertice++;
		
		spriteVertice->pos[X] = explosion->ss_boundaries[RIGHT];
		spriteVertice->pos[Y] = explosion->ss_boundaries[DOWN];
		spriteVertice->text[U] = expStage* SHRT_MAX/4 + SHRT_MAX/4;
		spriteVertice->text[V] = explosion->type* SHRT_MAX/4 + SHRT_MAX*1.f/4;
		spriteVertice->color[A] = expAlpha;
		spriteVertice++;
		
		spriteVertice->pos[X] = explosion->ss_boundaries[RIGHT];
		spriteVertice->pos[Y] = explosion->ss_boundaries[UP];
		spriteVertice->text[U] = expStage* SHRT_MAX/4 + SHRT_MAX/4;
		spriteVertice->text[V] = explosion->type* SHRT_MAX/4;
		spriteVertice->color[A] = expAlpha;
		spriteVertice++;

		numExplosionIndices  += 6;
		//numSpritesVertices += 4;
		
		explosion = explosion->next;
	}
	

}

particule_t* FX_GetFirstParticule(void)
{
	return rootParticule.next;
}


particule_t particuleNull;

particule_t* FX_GetParticule(vec2_t ss_position, vec2_t direction, float size, float travelDistance,uchar type, float colorType,int strech)
{
	particule_t* particule;
	int i;
	
	//This particule will never be rendered, it is the null void of non existance
	if (numFreeParticules == 0)
		return &particuleNull;
	
	numFreeParticules--;
	particule = freeParticules[numFreeParticules];

	//Insert after rootEnemy
	particule->next = rootParticule.next;
	
	if (particule->next != NULL)
		particule->next->prev = particule;
	
	particule->prev = &rootParticule;
	rootParticule.next = particule;
	
	particule->timeCounter = 0;
	particule->colorType = colorType;
	particule->type = type;
	
	//Now is time to shape it accordingly to it's position, diffs and size
	/*
			0   3
			1   2
	 */
		
	//ss_startBorders
	particule->ss_startBorders[0][X] =  (ss_position[X] - direction[Y] * size) * SS_W;
	particule->ss_startBorders[0][Y] =  (ss_position[Y] + direction[X] * size) * SS_H;	
	
	particule->ss_startBorders[3][X] = particule->ss_startBorders[0][X] +  direction[X] * size * SS_W;
	particule->ss_startBorders[3][Y] = particule->ss_startBorders[0][Y] +  direction[Y] * size * SS_H;
	
	
	particule->ss_startBorders[1][X] = (ss_position[X] + direction[Y] * size) * SS_W;
	particule->ss_startBorders[1][Y] = (ss_position[Y] - direction[X] * size) * SS_H;
	
	particule->ss_startBorders[2][X] = particule->ss_startBorders[1][X] + direction[X] * size * SS_W;
	particule->ss_startBorders[2][Y] = particule->ss_startBorders[1][Y] + direction[Y] * size * SS_H;
	
	
	
	//ss_endBorders;
	particule->ss_endBorders[0][X] = particule->ss_startBorders[0][X] + direction[X] *  SS_W * travelDistance;
	particule->ss_endBorders[0][Y] = particule->ss_startBorders[0][Y] + direction[Y] *  SS_H * travelDistance;
	
	particule->ss_endBorders[3][X] = particule->ss_endBorders[0][X] +   direction[X] * size*strech * SS_W;
	particule->ss_endBorders[3][Y] = particule->ss_endBorders[0][Y] +   direction[Y] * size*strech * SS_H;
	
	particule->ss_endBorders[1][X] = particule->ss_startBorders[1][X] + direction[X] * SS_W * travelDistance;
	particule->ss_endBorders[1][Y] = particule->ss_startBorders[1][Y] + direction[Y] * SS_H * travelDistance;
	
	particule->ss_endBorders[2][X] = particule->ss_endBorders[1][X] +   direction[X] * size*strech * SS_W;
	particule->ss_endBorders[2][Y] = particule->ss_endBorders[1][Y] +   direction[Y] * size*strech * SS_H;
	
	
		//diff c = a - b
	for (i=0; i < 4 ; i++) 
		vector2Subtract(particule->ss_endBorders[i],particule->ss_startBorders[i],particule->ss_diff[i]);

	
	return particule;
}

void FX_ReleaseParticule(particule_t* particule)
{
	freeParticules[numFreeParticules] = particule;
	numFreeParticules++;
	
	if (particule->prev != NULL)
		particule->prev->next = particule->next;
	
	if (particule->next != NULL)
		particule->next->prev = particule->prev;
	
	
	
}

#define PARTICULE_TTL 500
#define PARTICULE_RANGE_ALPHA 128
#define PARTICULE_END_ALPHA 50


#define PART_TEXT_WIDTH 128.0f
void FX_UpdateParticules(void)
{
	particule_t* particule;
	float alphaInterpolation;
	xf_sprite_t* particuleSpriteVertice;
	float interPolationF;
	
	particule = FX_GetFirstParticule();
	
	particuleSpriteVertice = particuleVertices;
	numParticulesIndices = 0;
	
	
	while (particule != NULL) 
	{
		
		// If the particule is exploding or imploding. 
		
		alphaInterpolation = particule->timeCounter/(float)PARTICULE_TTL;
		interPolationF = 1- alphaInterpolation;
		
		alphaInterpolation = particule->type - alphaInterpolation;
		alphaInterpolation = fabsf(alphaInterpolation);
		
		interPolationF = particule->type - interPolationF;
		interPolationF = fabsf(interPolationF);	
		/*
		 Weird x86 assembly 
		 
		 interPolationF = fabsf(particule->type - interPolationF);
		 0x00027d19  <+0139>  mov    -0xc(%ebp),%eax
		 0x00027d1c  <+0142>  movss  0x3c(%eax),%xmm0
		 0x00027d21  <+0147>  movaps %xmm0,%xmm1
		 0x00027d24  <+0150>  subss  -0x18(%ebp),%xmm1
		 0x00027d29  <+0155>  lea    0xd526(%ebx),%eax
		 0x00027d2f  <+0161>  movaps (%eax),%xmm0
		 0x00027d32  <+0164>  andps  %xmm1,%xmm0
		 0x00027d35  <+0167>  movss  %xmm0,-0x18(%ebp)
		 
			
							VS
		 
		 
		 interPolationF = particule->type - interPolationF;
		 0x00027d19  <+0139>  mov    -0xc(%ebp),%eax
		 0x00027d1c  <+0142>  movss  0x3c(%eax),%xmm0
		 0x00027d21  <+0147>  subss  -0x18(%ebp),%xmm0
		 0x00027d26  <+0152>  movss  %xmm0,-0x18(%ebp)
		 
		 interPolationF = fabsf(interPolationF)  ;
		 0x00027d2b  <+0157>  lea    0xd526(%ebx),%eax
		 0x00027d31  <+0163>  movaps (%eax),%xmm0
		 0x00027d34  <+0166>  andl   $0x7fffffff,-0x18(%ebp)
		 
		 */
		
		//alphaInterpolation = PARTICULE_START_ALPHA + interPolationF * (PARTICULE_END_ALPHA - PARTICULE_START_ALPHA);
		//if (alphaInterpolation < 0 )
		//	alphaInterpolation = 0;
		
		// Simulate deceleration via power factor
		interPolationF = powf(interPolationF,4);
		interPolationF = 1- interPolationF;
		
		//Same kind of simulation for alpha
		alphaInterpolation = powf(alphaInterpolation,4);
		alphaInterpolation = 1-alphaInterpolation;
		alphaInterpolation = alphaInterpolation*PARTICULE_RANGE_ALPHA + PARTICULE_END_ALPHA;
		alphaInterpolation = fabsf(alphaInterpolation);
		//alphaInterpolation = 255;
		
		/*
		 0   3
		 1   2
		 */
		
		particuleSpriteVertice->pos[X] = particule->ss_startBorders[0][X] + interPolationF * particule->ss_diff[0][X];
		particuleSpriteVertice->pos[Y] = particule->ss_startBorders[0][Y] + interPolationF * particule->ss_diff[0][Y];
		particuleSpriteVertice->color[A] = alphaInterpolation;
		particuleSpriteVertice->text[U] = 48/PART_TEXT_WIDTH * SHRT_MAX + particule->colorType * 16/PART_TEXT_WIDTH * SHRT_MAX;
		particuleSpriteVertice++;
		
		particuleSpriteVertice->pos[X] = particule->ss_startBorders[1][X] + interPolationF * particule->ss_diff[1][X];
		particuleSpriteVertice->pos[Y] = particule->ss_startBorders[1][Y] + interPolationF * particule->ss_diff[1][Y];
		particuleSpriteVertice->color[A] = alphaInterpolation;
		particuleSpriteVertice->text[U] = 48/PART_TEXT_WIDTH * SHRT_MAX + particule->colorType * 16/PART_TEXT_WIDTH * SHRT_MAX;
		particuleSpriteVertice++;
		
		particuleSpriteVertice->pos[X] = particule->ss_startBorders[2][X] + interPolationF * particule->ss_diff[2][X];
		particuleSpriteVertice->pos[Y] = particule->ss_startBorders[2][Y] + interPolationF * particule->ss_diff[2][Y];
		particuleSpriteVertice->color[A] = alphaInterpolation;
		particuleSpriteVertice->text[U] = 48/PART_TEXT_WIDTH * SHRT_MAX + particule->colorType * 16/PART_TEXT_WIDTH * SHRT_MAX + 16/PART_TEXT_WIDTH * SHRT_MAX;
		particuleSpriteVertice++;
		
		particuleSpriteVertice->pos[X] = particule->ss_startBorders[3][X] + interPolationF * particule->ss_diff[3][X];
		particuleSpriteVertice->pos[Y] = particule->ss_startBorders[3][Y] + interPolationF * particule->ss_diff[3][Y];
		particuleSpriteVertice->color[A] = alphaInterpolation;
		particuleSpriteVertice->text[U] = 48/PART_TEXT_WIDTH * SHRT_MAX + particule->colorType * 16/PART_TEXT_WIDTH * SHRT_MAX + 16/PART_TEXT_WIDTH * SHRT_MAX;
		particuleSpriteVertice++;
		
		
		numParticuleVertices += 4;
		numParticulesIndices+=6;
		
		
		if (particule->timeCounter > PARTICULE_TTL)
			FX_ReleaseParticule(particule);
		
		particule->timeCounter += timediff;
		particule = particule->next;
	}
}




/////// SMOKE/////////////////////////////
/////// SMOKE/////////////////////////////
/////// SMOKE/////////////////////////////
/////// SMOKE/////////////////////////////


#define SMOKE_SIZE 0.2
#define SMOKE_MOVING_SPEED (-0.4/1000.0f)
void FX_UpdateSmoke(void)
{
	smoke_t* smoke;
	xf_colorless_sprite_t* smokeVertice;
	
	smoke = FX_GetFirstSmoke();
	
	while (smoke) 
	{
		smoke->timeCounter += timediff;
		
		if (smoke->timeCounter > smoke->ttl)
			FX_ReleaseSmoke(smoke);
			
		smokeVertice = &smokeVertices[smoke->vertexStart];
		
		// Smoke is moving down
		smokeVertice->pos[Y] += timediff * SMOKE_MOVING_SPEED * SS_H;
		smokeVertice++;
		smokeVertice->pos[Y] += timediff * SMOKE_MOVING_SPEED * SS_H;
		smokeVertice++;
		smokeVertice->pos[Y] += timediff * SMOKE_MOVING_SPEED * SS_H;
		smokeVertice++;
		smokeVertice->pos[Y] += timediff * SMOKE_MOVING_SPEED * SS_H;
		//smokeVertice++;

		
		
		smoke = smoke->next;
	}
}

// Just a convenient array to flip explosion and fake some diversity via texture coordinate
// 0  2 
// 1  3


// 1  0
// 3  2

/*
const ushort explosionDiv[4][6] =
{
	{1,3,0,0,3,2},
	{0,2,1,1,2,3},
	{3,1,2,2,1,0},
	{1,0,3,3,0,2}
};
*/
void FX_PrepareSmokeSprites(void)
{
	int frameId;
	int x,y;
	smoke_t* smoke;
	xf_colorless_sprite_t* smokeVertice;
	ushort* smokeIndice_p;
	smoke = FX_GetFirstSmoke();
	
	numSmokeIndices = 0;
	smokeIndice_p = smokeIndices;
	
	//const ushort* flippingArray;
	
	// 0  2 
	// 1  3
	while (smoke) 
	{
		// Update vertexArray with proper texture coordinates
		frameId = smoke->timeCounter / (float)smoke->ttl * 63;
		//x = frameId % 8 ;
		x = frameId & 7 ;
		y = frameId / 8 ;
		
		smokeVertice = &smokeVertices[smoke->vertexStart];
		
		//  0  1
		//  1  3
		smokeVertice->text[X] =  x * SHRT_MAX/8 + smoke->text_coo[0][X];
		smokeVertice->text[Y] =  y * SHRT_MAX/8 + smoke->text_coo[0][Y];
		smokeVertice++;
		
		smokeVertice->text[X] =  x * SHRT_MAX/8 + smoke->text_coo[1][X];
		smokeVertice->text[Y] =  y * SHRT_MAX/8 + smoke->text_coo[1][Y];
		smokeVertice++;
		
		smokeVertice->text[X] =  x * SHRT_MAX/8 + smoke->text_coo[2][X];
		smokeVertice->text[Y] =  y * SHRT_MAX/8 + smoke->text_coo[2][Y];
		smokeVertice++;
		
		smokeVertice->text[X] =  x * SHRT_MAX/8 + smoke->text_coo[3][X];
		smokeVertice->text[Y] =  y * SHRT_MAX/8 + smoke->text_coo[3][Y];
		
		
		//Also need to update the vertice indices
		
		//flippingArray = explosionDiv[smoke->type];
		
		*smokeIndice_p		= smoke->vertexStart+1 ;
		*(smokeIndice_p+1)	= smoke->vertexStart+3 ;
		*(smokeIndice_p+2)	= smoke->vertexStart+0 ;
		*(smokeIndice_p+3)	= smoke->vertexStart+3 ;
		*(smokeIndice_p+4)	= smoke->vertexStart+0 ;
		*(smokeIndice_p+5)	= smoke->vertexStart+2 ;
		numSmokeIndices+=6;
		smokeIndice_p+=6;
		
		
		//Need to slightly update position
		
		smoke = smoke->next;
	}
	
	//printf("numSmokeIndices=%d.\n",numSmokeIndices);
}


smoke_t nullSmoke;
smoke_t* FX_GetSmoke(vec2_t ss_position,float ss_sizeX, float ss_sizeY)
{
	smoke_t* smoke;
	char type ;
	char shouldFlip ;
	float rotation;
	int i;
	ushort tmp;
	xf_colorless_sprite_t* smokeVertice;
	
	if (numFreeSmokes == 0)
		return &nullSmoke;
	
	
	numFreeSmokes--;
	smoke = freeSmokes[numFreeSmokes];
	
	//Insert after root Smoke
	smoke->next = rootSmoke.next;
	
	if (smoke->next != NULL)
		smoke->next->prev = smoke;
	
	smoke->prev = &rootSmoke;
	rootSmoke.next = smoke;
	
	smoke->timeCounter = 0;
	
	// ???? WTF ?????? Ugly hack...just want to ship the game now
	ss_position[Y] += 0.2;
	
	
	//printf("smoke t=%d.\n",smoke->type);
	
	if (ss_sizeX == 0)
	{
		ss_sizeX = SMOKE_SIZE;
		ss_sizeY = SMOKE_SIZE;
	}
	
	//Randomize slightly size
	ss_sizeX += ss_sizeX * rand()/RAND_MAX*0.5f;
	ss_sizeY += ss_sizeY * rand()/RAND_MAX*0.5f;
	
	//Randomize slightly ttl
	smoke->ttl = 1000;
	smoke->ttl += smoke->ttl * rand()/RAND_MAX*0.5f;//0-0.1
	
	// Generate start and end texture positions.
	// Disregard ss_sizeX/Y for now, consum a LOT of bloody fillrate but maybe it will do it anyway.
	smokeVertice = &smokeVertices[smoke->vertexStart];
	
	// 0  2 
	// 1  3

	smokeVertice->pos[X] = (ss_position[X] - ss_sizeX) * SS_W ;
	smokeVertice->pos[Y] = (ss_position[Y] + ss_sizeY) * SS_W  ;
	smokeVertice++;
	
	smokeVertice->pos[X] = (ss_position[X] - ss_sizeX) * SS_W  ;
	smokeVertice->pos[Y] = (ss_position[Y] - ss_sizeY) * SS_W  ;
	smokeVertice++;
	
	smokeVertice->pos[X] = (ss_position[X] + ss_sizeX) * SS_W  ;
	smokeVertice->pos[Y] = (ss_position[Y] + ss_sizeY) * SS_W  ;
	smokeVertice++;
	
	
	smokeVertice->pos[X] = (ss_position[X] + ss_sizeX) * SS_W  ;
	smokeVertice->pos[Y] = (ss_position[Y] - ss_sizeY) * SS_W  ;
	
	
	//Save textures coordinates
	//Randomize upside-down and leftside-right
	type = rand() & 3 ;
	rotation = type * M_PI/2.0f ;
	
	smoke->text_coo[0][X] = -SHRT_MAX/16;
	smoke->text_coo[0][Y] = -SHRT_MAX/16;
	
	smoke->text_coo[1][X] = -SHRT_MAX/16;
	smoke->text_coo[1][Y] =  SHRT_MAX/16;
	
	smoke->text_coo[2][X] =  SHRT_MAX/16;
	smoke->text_coo[2][Y] = -SHRT_MAX/16;
	
	smoke->text_coo[3][X] =  SHRT_MAX/16;
	smoke->text_coo[3][Y] =  SHRT_MAX/16;
	
	//rotate the sucker
	for (i= 0 ; i < 4 ; i++)
	{
		tmp = smoke->text_coo[i][X] ;
		smoke->text_coo[i][X] =  tmp * cosf(rotation) + smoke->text_coo[i][Y] * sinf(rotation);
		smoke->text_coo[i][Y] = -tmp * sinf(rotation) + smoke->text_coo[i][Y] * cosf(rotation);
	}
	
	for (i= 0 ; i < 4 ; i++)
	{
		smoke->text_coo[i][X] += SHRT_MAX/16;
		smoke->text_coo[i][Y] += SHRT_MAX/16;
	}
		
	// flip it
	shouldFlip = rand() & 1 ;
	if (shouldFlip) //Yes we are flipping
	{ 
		if (type == 0 || type == 2) //vertical flip 
		{
			// 0  2     =>      1   3
			// 1  3             0   2
			tmp = smoke->text_coo[0][X];
			smoke->text_coo[0][X] = smoke->text_coo[1][X];
			smoke->text_coo[1][X] = tmp;
			
			tmp = smoke->text_coo[0][Y];
			smoke->text_coo[0][Y] = smoke->text_coo[1][Y];
			smoke->text_coo[1][Y] = tmp;
			

			tmp = smoke->text_coo[2][X];
			smoke->text_coo[2][X] = smoke->text_coo[3][X];
			smoke->text_coo[3][X] = tmp;
			
			tmp = smoke->text_coo[2][Y];
			smoke->text_coo[2][Y] = smoke->text_coo[3][Y];
			smoke->text_coo[3][Y] = tmp;
		}
		else 
		{
			// 0  2     =>      2   0
			// 1  3             3   1		
			tmp = smoke->text_coo[0][X];
			smoke->text_coo[0][X] = smoke->text_coo[2][X];
			smoke->text_coo[2][X] = tmp;
			
			tmp = smoke->text_coo[0][Y];
			smoke->text_coo[0][Y] = smoke->text_coo[2][Y];
			smoke->text_coo[2][Y] = tmp;
			
			
			tmp = smoke->text_coo[1][X];
			smoke->text_coo[1][X] = smoke->text_coo[3][X];
			smoke->text_coo[3][X] = tmp;
			
			tmp = smoke->text_coo[1][Y];
			smoke->text_coo[1][Y] = smoke->text_coo[3][Y];
			smoke->text_coo[3][Y] = tmp;		
			
		}

	}
	
	return smoke;
}

smoke_t* FX_GetFirstSmoke(void)
{
	return rootSmoke.next;
}

void FX_ReleaseSmoke(smoke_t* smoke)
{
	//printf("[FX_ReleaseSmoke] numFreeSmokes=%d\n",numFreeSmokes);
	freeSmokes[numFreeSmokes] = smoke;
	numFreeSmokes++;
	
	if (smoke->prev != NULL)
		smoke->prev->next = smoke->next;
	
	if (smoke->next != NULL)
		smoke->next->prev = smoke->prev;
}